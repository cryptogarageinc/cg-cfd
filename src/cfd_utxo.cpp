// Copyright 2019 CryptoGarage
/**
 * @file cfd_utxo.cpp
 *
 * @brief UTXO操作の関連クラスの実装ファイル
 */
#include <algorithm>
#include <cassert>
#include <string>
#include <vector>

#include "cfd/cfd_utxo.h"

#include "cfd/cfd_common.h"
#include "cfdcore/cfdcore_address.h"
#include "cfdcore/cfdcore_amount.h"
#include "cfdcore/cfdcore_coin.h"
#include "cfdcore/cfdcore_exception.h"
#include "cfdcore/cfdcore_logger.h"
#include "cfdcore/cfdcore_script.h"
#ifndef CFD_DISABLE_ELEMENTS
#include "cfdcore/cfdcore_elements_transaction.h"
#endif  // CFD_DISABLE_ELEMENTS

namespace cfd {

using cfd::core::Amount;
using cfd::core::BlockHash;
using cfd::core::CfdError;
using cfd::core::CfdException;
using cfd::core::kMaxAmount;
using cfd::core::Script;
using cfd::core::Txid;
#ifndef CFD_DISABLE_ELEMENTS
using cfd::core::ConfidentialAssetId;
#endif  // CFD_DISABLE_ELEMENTS
using cfd::core::logger::warn;

// -----------------------------------------------------------------------------
// Inner definitions
// -----------------------------------------------------------------------------

static constexpr const size_t kBnBTotalTries = 100000;

// Descending order comparator
struct {
  bool operator()(const Utxo& a, const Utxo& b) const {
    return a.effective_value > b.effective_value;
  }
} descending;

// -----------------------------------------------------------------------------
// CoinSelectionOption
// -----------------------------------------------------------------------------
CoinSelectionOption::CoinSelectionOption() {
  // do nothing
}

bool CoinSelectionOption::IsUseBnB() const { return use_bnb_; }

size_t CoinSelectionOption::GetChangeOutputSize() const {
  return change_output_size_;
}

size_t CoinSelectionOption::GetChangeSpendSize() const {
  return change_spend_size_;
}

uint64_t CoinSelectionOption::GetEffectiveFeeBaserate() const {
  return effective_fee_baserate_;
}

size_t CoinSelectionOption::GetTxNoInputsSize() const {
  return tx_noinputs_size_;
}

void CoinSelectionOption::SetUseBnB(bool use_bnb) { use_bnb_ = use_bnb; }

void CoinSelectionOption::SetChangeOutputSize(size_t size) {
  change_output_size_ = size;
}

void CoinSelectionOption::SetChangeSpendSize(size_t size) {
  change_spend_size_ = size;
}

void CoinSelectionOption::SetEffectiveFeeBaserate(uint64_t baserate) {
  effective_fee_baserate_ = baserate;
}

void CoinSelectionOption::SetTxNoInputsSize(size_t size) {
  tx_noinputs_size_ = size;
}

#ifndef CFD_DISABLE_ELEMENTS
ConfidentialAssetId CoinSelectionOption::GetFeeAsset() const {
  return fee_asset_;
}

void CoinSelectionOption::SetFeeAsset(const ConfidentialAssetId& asset) {
  fee_asset_ = asset;
}
#endif  // CFD_DISABLE_ELEMENTS

// -----------------------------------------------------------------------------
// CoinSelection
// -----------------------------------------------------------------------------
CoinSelection::CoinSelection() : use_bnb_(true) {
  // do nothing
}

CoinSelection::CoinSelection(bool use_bnb) : use_bnb_(use_bnb) {
  // do nothing
}

std::vector<Utxo> CoinSelection::SelectCoinsMinConf(
    const Amount& target_value, const std::vector<Utxo>& utxos,
    const UtxoFilter& filter, const CoinSelectionOption& option_params,
    Amount* select_value, Amount* fee_value) const {
  // FIXME
  return std::vector<Utxo>();
}

std::vector<Utxo> CoinSelection::SelectCoinsBnB(
    const Amount& target_value, const std::vector<Utxo>& utxos,
    const Amount& cost_of_change, const Amount& not_input_fees,
    Amount* select_value) const {
  std::vector<Utxo> results;
  Amount curr_value = Amount::CreateBySatoshiAmount(0);

  std::vector<bool> curr_selection;
  curr_selection.reserve(utxos.size());
  Amount actual_target = not_input_fees + target_value;

  // Calculate curr_available_value
  Amount curr_available_value = Amount::CreateBySatoshiAmount(0);
  for (const Utxo utxo : utxos) {
    // Assert that this utxo is not negative. It should never be negative,
    //  effective value calculation should have removed it
    assert(utxo.effective_value > 0);
    curr_available_value += utxo.effective_value;
  }
  if (curr_available_value < actual_target) {
    // not enough amount
    warn(
        CFD_LOG_SOURCE,
        "Failed to SelectCoinsBnB. Not enough utxos."
        ": curr_available_value={}, actual_target={}",
        curr_available_value, actual_target);
    throw CfdException(
        CfdError::kCfdIllegalStateError,
        "Failed to select coin. Not enough utxos.");
  }

  // Sort the utxos
  std::sort(utxos.begin(), utxos.end(), descending);

  Amount curr_waste = Amount::CreateBySatoshiAmount(0);
  std::vector<bool> best_selection;
  Amount best_waste = Amount::CreateBySatoshiAmount(kMaxAmount);

  // Depth First search loop for choosing the UTXOs
  for (size_t i = 0; i < kBnBTotalTries; ++i) {
    // Conditions for starting a backtrack
    bool backtrack = false;
    if (curr_value + curr_available_value <
            actual_target ||  // Cannot possibly reach target with the amount remaining in the curr_available_value.
        curr_value >
            actual_target +
                cost_of_change ||  // Selected value is out of range, go back and try other branch
        (curr_waste > best_waste &&
         (utxos.at(0).fee - utxos.at(0).long_term_fee) >
             0)) {  // Don't select things which we know will be more wasteful if the waste is increasing
      backtrack = true;
    } else if (
        curr_value >= actual_target) {  // Selected value is within range
      curr_waste +=
          (curr_value -
           actual_target);  // This is the excess value which is added to the waste for the below comparison
      // Adding another UTXO after this check could bring the waste down if the long term fee is higher than the current fee.
      // However we are not going to explore that because this optimization for the waste is only done when we have hit our target
      // value. Adding any more UTXOs will be just burning the UTXO; it will go entirely to fees. Thus we aren't going to
      // explore any more UTXOs to avoid burning money like that.
      if (curr_waste <= best_waste) {
        best_selection = curr_selection;
        best_selection.resize(utxos.size());
        best_waste = curr_waste;
      }
      curr_waste -=
          (curr_value -
           actual_target);  // Remove the excess value as we will be selecting different coins now
      backtrack = true;
    }

    // Backtracking, moving backwards
    if (backtrack) {
      // Walk backwards to find the last included UTXO that still needs to have its omission branch traversed.
      while (!curr_selection.empty() && !curr_selection.back()) {
        curr_selection.pop_back();
        curr_available_value +=
            utxos.at(curr_selection.size()).effective_value;
      }

      if (curr_selection
              .empty()) {  // We have walked back to the first utxo and no branch is untraversed. All solutions searched
        break;
      }

      // Output was included on previous iterations, try excluding now.
      curr_selection.back() = false;
      const Utxo& utxo = utxos.at(curr_selection.size() - 1);
      curr_value -= utxo.effective_value;
      curr_waste -= utxo.fee - utxo.long_term_fee;
    } else {  // Moving forwards, continuing down this branch
      const Utxo& utxo = utxos.at(curr_selection.size());

      // Remove this utxo from the curr_available_value utxo amount
      curr_available_value -= utxo.effective_value;

      // Avoid searching a branch if the previous UTXO has the same value and same waste and was excluded. Since the ratio of fee to
      // long term fee is the same, we only need to check if one of those values match in order to know that the waste is the same.
      if (!curr_selection.empty() && !curr_selection.back() &&
          utxo.effective_value ==
              utxos.at(curr_selection.size() - 1).effective_value &&
          utxo.fee == utxos.at(curr_selection.size() - 1).fee) {
        curr_selection.push_back(false);
      } else {
        // Inclusion branch first (Largest First Exploration)
        curr_selection.push_back(true);
        curr_value += utxo.effective_value;
        curr_waste += utxo.fee - utxo.long_term_fee;
      }
    }
  }

  // Check for solution
  if (best_selection.empty()) {
    // not enough amount
    warn(
        CFD_LOG_SOURCE,
        "Failed to SelectCoinsBnB. Cannot find best solution.");
    throw CfdException(
        CfdError::kCfdIllegalStateError,
        "Failed to SelectCoinsBnB. Cannot find best solution.");
  }

  // Set output set
  *select_value = Amount::CreateBySatoshiAmount(0);
  for (size_t i = 0; i < best_selection.size(); ++i) {
    if (best_selection.at(i)) {
      results.push_back(utxos.at(i));
      *select_value += static_cast<int64_t>(utxos.at(i).amount);
    }
  }

  return results;
}

std::vector<Utxo> CoinSelection::KnapsackSolver(
    const Amount& target_value, const std::vector<Utxo>& utxos,
    Amount* select_value) const {
  // FIXME
  return std::vector<Utxo>();
}

void CoinSelection::ConvertToUtxo(
    uint64_t block_height, const BlockHash& block_hash, const Txid& txid,
    uint32_t vout, const Script& locking_script,
    const std::string& output_descriptor, const Amount& amount,
    const void* binary_data, Utxo* utxo) {
  // FIXME
}

#ifndef CFD_DISABLE_ELEMENTS
void CoinSelection::ConvertToUtxo(
    uint64_t block_height, const BlockHash& block_hash, const Txid& txid,
    uint32_t vout, const Script& locking_script,
    const std::string& output_descriptor, const Amount& amount,
    const ConfidentialAssetId& asset, const void* binary_data, Utxo* utxo) {
  // FIXME
}
#endif  // CFD_DISABLE_ELEMENTS

}  // namespace cfd
