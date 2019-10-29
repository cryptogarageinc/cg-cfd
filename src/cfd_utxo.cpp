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
using cfd::core::RandomNumberUtil;
using cfd::core::Script;
using cfd::core::Txid;
#ifndef CFD_DISABLE_ELEMENTS
using cfd::core::ConfidentialAssetId;
#endif  // CFD_DISABLE_ELEMENTS
using cfd::core::logger::warn;

// -----------------------------------------------------------------------------
// Inner definitions
// -----------------------------------------------------------------------------
//! SelectCoinsBnBの最大繰り返し回数
static constexpr const size_t kBnBMaxTotalTries = 100000;

//! KnapsackSolver ApproximateBestSubsetの繰り返し回数
static constexpr const int kApproximateBestSubsetIterations = 100000;

//!  Change最小値
static constexpr const uint64_t kMinChange = 1000000;  // MIN_CHANGE

/**
 * 収集額に最も近い合計額となるUTXO一覧を決定する
 * @param[in]  utxos          収集額より小さいUTXO一覧
 * @param[in]  n_total_value  utxo一覧の合計額
 * @param[in]  n_target_value 収集額
 * @param[out] vf_best        収集対象フラグ一覧
 * @param[out] n_best         収集額に最も近い合計額
 * @param[in]  iterations     繰り返し数
 */
void ApproximateBestSubset(
    const std::vector<const Utxo*>& utxos, uint64_t n_total_value,
    uint64_t n_target_value, std::vector<char>* vf_best, uint64_t* n_best,
    int iterations = kApproximateBestSubsetIterations) {
  if (vf_best == nullptr || n_best == nullptr) {
    warn(CFD_LOG_SOURCE, "Outparameter(select_value) is nullptr.");
    throw CfdException(
        CfdError::kCfdIllegalArgumentError,
        "Failed to select coin. Outparameter is nullptr.");
  }

  std::vector<char> vf_includes;
  *n_best = n_total_value;

  for (int n_rep = 0; n_rep < iterations && *n_best != n_target_value;
       n_rep++) {
    vf_includes.assign(utxos.size(), false);
    uint64_t n_total = 0;
    bool is_reached_target = false;
    std::vector<bool> randomize_cache;
    for (int n_pass = 0; n_pass < 2 && !is_reached_target; n_pass++) {
      for (unsigned int i = 0; i < utxos.size(); i++) {
        // The solver here uses a randomized algorithm,
        // the randomness serves no real security purpose but is just
        // needed to prevent degenerate behavior and it is important
        // that the rng is fast. We do not use a constant random sequence,
        // because there may be some privacy improvement by making
        // the selection random.
        bool rand_bool = RandomNumberUtil::GetRandomBool(&randomize_cache);
        if (n_pass == 0 ? rand_bool : !vf_includes[i]) {
          n_total += utxos[i]->amount;
          vf_includes[i] = true;
          if (n_total >= n_target_value) {
            is_reached_target = true;
            if (n_total < *n_best) {
              *n_best = n_total;
              *vf_best = vf_includes;
            }
            n_total -= utxos[i]->amount;
            vf_includes[i] = false;
          }
        }
      }
    }
  }
}

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
  if (select_value == nullptr) {
    warn(CFD_LOG_SOURCE, "Outparameter(select_value) is nullptr.");
    throw CfdException(
        CfdError::kCfdIllegalArgumentError,
        "Failed to select coin. Outparameter is nullptr.");
  }

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
        ": curr_available_value={},"
        "actual_target={}",
        curr_available_value.GetSatoshiValue(),
        actual_target.GetSatoshiValue());
    throw CfdException(
        CfdError::kCfdIllegalStateError,
        "Failed to select coin. Not enough utxos.");
  }

  // copy utxo pointer
  std::vector<const Utxo*> p_utxos(utxos.size());
  for (const Utxo& utxo : utxos) {
    p_utxos.push_back(&utxo);
  }
  // Sort the utxos
  std::sort(p_utxos.begin(), p_utxos.end(), [](const Utxo* a, const Utxo* b) {
    return a->effective_value > b->effective_value;
  });

  Amount curr_waste = Amount::CreateBySatoshiAmount(0);
  std::vector<bool> best_selection;
  Amount best_waste = Amount::CreateBySatoshiAmount(kMaxAmount);

  // Depth First search loop for choosing the UTXOs
  for (size_t i = 0; i < kBnBMaxTotalTries; ++i) {
    // Conditions for starting a backtrack
    bool backtrack = false;
    if (curr_value + curr_available_value <
            actual_target ||  //NOLINT Cannot possibly reach target with the amount remaining in the curr_available_value.
        curr_value >
            actual_target +
                cost_of_change ||  //NOLINT Selected value is out of range, go back and try other branch
        (curr_waste > best_waste &&
         (p_utxos.at(0)->fee - p_utxos.at(0)->long_term_fee) >
             0)) {  //NOLINT Don't select things which we know will be more wasteful if the waste is increasing
      backtrack = true;
    } else if (
        curr_value >= actual_target) {  //NOLINT Selected value is within range
      curr_waste +=
          (curr_value -
           actual_target);  //NOLINT This is the excess value which is added to the waste for the below comparison
      //NOLINT Adding another UTXO after this check could bring the waste down if the long term fee is higher than the current fee.
      //NOLINT However we are not going to explore that because this optimization for the waste is only done when we have hit our target
      //NOLINT value. Adding any more UTXOs will be just burning the UTXO; it will go entirely to fees. Thus we aren't going to
      //NOLINT explore any more UTXOs to avoid burning money like that.
      if (curr_waste <= best_waste) {
        best_selection = curr_selection;
        best_selection.resize(p_utxos.size());
        best_waste = curr_waste;
      }
      curr_waste -=
          (curr_value -
           actual_target);  //NOLINT Remove the excess value as we will be selecting different coins now
      backtrack = true;
    }

    // Backtracking, moving backwards
    if (backtrack) {
      //NOLINT Walk backwards to find the last included UTXO that still needs to have its omission branch traversed.
      while (!curr_selection.empty() && !curr_selection.back()) {
        curr_selection.pop_back();
        curr_available_value +=
            p_utxos.at(curr_selection.size())->effective_value;
      }

      if (curr_selection
              .empty()) {  //NOLINT We have walked back to the first utxo and no branch is untraversed. All solutions searched
        break;
      }

      // Output was included on previous iterations, try excluding now.
      curr_selection.back() = false;
      const Utxo* utxo = p_utxos.at(curr_selection.size() - 1);
      curr_value -= utxo->effective_value;
      curr_waste -= utxo->fee - utxo->long_term_fee;
    } else {  // Moving forwards, continuing down this branch
      const Utxo* utxo = p_utxos.at(curr_selection.size());

      // Remove this utxo from the curr_available_value utxo amount
      curr_available_value -= utxo->effective_value;

      // NOLINT Avoid searching a branch if the previous UTXO has the same value and same waste and was excluded. Since the ratio of fee to
      // NOLINT long term fee is the same, we only need to check if one of those values match in order to know that the waste is the same.
      if (!curr_selection.empty() && !curr_selection.back() &&
          utxo->effective_value ==
              p_utxos.at(curr_selection.size() - 1)->effective_value &&
          utxo->fee == p_utxos.at(curr_selection.size() - 1)->fee) {
        curr_selection.push_back(false);
      } else {
        // Inclusion branch first (Largest First Exploration)
        curr_selection.push_back(true);
        curr_value += utxo->effective_value;
        curr_waste += utxo->fee - utxo->long_term_fee;
      }
    }
  }

  // Check for solution
  if (best_selection.empty()) {
    return results;
  }

  // Set output set
  *select_value = Amount::CreateBySatoshiAmount(0);
  for (size_t i = 0; i < best_selection.size(); ++i) {
    if (best_selection.at(i)) {
      results.push_back(*(p_utxos.at(i)));
      *select_value += static_cast<int64_t>(p_utxos.at(i)->amount);
    }
  }

  return results;
}

std::vector<Utxo> CoinSelection::KnapsackSolver(
    const Amount& target_value, const std::vector<Utxo>& utxos,
    Amount* select_value) const {
  std::vector<Utxo> ret_utxos;

  if (select_value == nullptr) {
    warn(CFD_LOG_SOURCE, "Outparameter(select_value) is nullptr.");
    throw CfdException(
        CfdError::kCfdIllegalArgumentError,
        "Failed to select coin. Outparameter is nullptr.");
  }

  // List of values less than target
  Utxo lowest_larger;
  std::vector<const Utxo*> applicable_groups;
  uint64_t n_total = 0;
  uint64_t n_target = target_value.GetSatoshiValue();

  std::vector<uint32_t> indexes =
      RandomNumberUtil::GetRandomIndexes(utxos.size());

  for (size_t index = 0; index < indexes.size(); ++index) {
    if (utxos[index].amount == n_target) {
      ret_utxos.push_back(utxos[index]);
      *select_value = Amount::CreateBySatoshiAmount(utxos[index].amount);
      return ret_utxos;

    } else if (utxos[index].amount < n_target + kMinChange) {
      applicable_groups.push_back(&utxos[index]);
      n_total += utxos[index].amount;

    } else if (
        lowest_larger.amount == 0 ||
        utxos[index].amount < lowest_larger.amount) {
      lowest_larger = utxos[index];
    }
  }

  if (n_total == n_target) {
    uint64_t ret_value = 0;
    for (const auto& utxo : applicable_groups) {
      ret_utxos.push_back(*utxo);
      ret_value += utxo->amount;
    }
    *select_value = Amount::CreateBySatoshiAmount(ret_value);
    return ret_utxos;
  }

  if (n_total < n_target) {
    if (lowest_larger.amount == 0) {
      warn(
          CFD_LOG_SOURCE, "insufficient funds. total:{} target:{}", n_total,
          n_target);
      throw CfdException(
          CfdError::kCfdIllegalStateError, "insufficient funds.");
    }

    ret_utxos.push_back(lowest_larger);
    *select_value = Amount::CreateBySatoshiAmount(lowest_larger.amount);
    return ret_utxos;
  }

  std::sort(
      applicable_groups.begin(), applicable_groups.end(),
      [](const Utxo* a, const Utxo* b) {
        return a->effective_value > b->effective_value;
      });
  std::vector<char> vf_best;
  uint64_t n_best;

  ApproximateBestSubset(
      applicable_groups, n_total, n_target, &vf_best, &n_best);
  if (n_best != n_target && n_total >= n_target + kMinChange) {
    ApproximateBestSubset(
        applicable_groups, n_total, (n_target + kMinChange), &vf_best,
        &n_best);
  }

  // NOLINT If we have a bigger coin and (either the stochastic approximation didn't find a good solution,
  // NOLINT                                or the next bigger coin is closer), return the bigger coin
  if (lowest_larger.amount != 0 &&
      ((n_best != n_target && n_best < n_target + kMinChange) ||
       lowest_larger.amount <= n_best)) {
    ret_utxos.push_back(lowest_larger);
    *select_value = Amount::CreateBySatoshiAmount(lowest_larger.amount);

  } else {
    uint64_t ret_value = 0;
    for (unsigned int i = 0; i < applicable_groups.size(); i++) {
      if (vf_best[i]) {
        ret_utxos.push_back(*applicable_groups[i]);
        ret_value += applicable_groups[i]->amount;
      }
    }
    *select_value = Amount::CreateBySatoshiAmount(ret_value);
  }
  return ret_utxos;
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
