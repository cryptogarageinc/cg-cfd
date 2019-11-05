// Copyright 2019 CryptoGarage
/**
 * @file cfd_utxo.cpp
 *
 * @brief UTXO操作の関連クラスの実装ファイル
 */
#include <algorithm>
#include <cassert>
#include <cmath>
#include <string>
#include <vector>

#include "cfd/cfd_utxo.h"

#include "cfd/cfd_common.h"
#include "cfd/cfd_fee.h"
#include "cfdcore/cfdcore_address.h"
#include "cfdcore/cfdcore_amount.h"
#include "cfdcore/cfdcore_coin.h"
#include "cfdcore/cfdcore_exception.h"
#include "cfdcore/cfdcore_logger.h"
#include "cfdcore/cfdcore_script.h"
#include "cfdcore/cfdcore_transaction_common.h"
#ifndef CFD_DISABLE_ELEMENTS
#include "cfdcore/cfdcore_elements_transaction.h"
#endif  // CFD_DISABLE_ELEMENTS

namespace cfd {

using cfd::core::AbstractTransaction;
using cfd::core::AddressType;
using cfd::core::Amount;
using cfd::core::BlockHash;
using cfd::core::CfdError;
using cfd::core::CfdException;
using cfd::core::kMaxAmount;
using cfd::core::RandomNumberUtil;
using cfd::core::Script;
using cfd::core::Txid;
using cfd::core::TxIn;
#ifndef CFD_DISABLE_ELEMENTS
using cfd::core::ConfidentialAssetId;
using cfd::core::ConfidentialTxIn;
using cfd::core::ConfidentialTxOut;
using cfd::core::ConfidentialTxOutReference;
using cfd::core::ConfidentialValue;
#endif  // CFD_DISABLE_ELEMENTS
using cfd::core::logger::info;
using cfd::core::logger::warn;

// -----------------------------------------------------------------------------
// Inner definitions
// -----------------------------------------------------------------------------
//! SelectCoinsBnBの最大繰り返し回数
static constexpr const size_t kBnBMaxTotalTries = 100000;

//! KnapsackSolver ApproximateBestSubsetの繰り返し回数
static constexpr const int kApproximateBestSubsetIterations = 100000;

//! Change最小値
static constexpr const uint64_t kMinChange = 1000000;  // MIN_CHANGE

//! LongTerm fee rate default (20.0)
static constexpr const uint64_t kDefaultLongTermFeeRate = 20000;

// -----------------------------------------------------------------------------
// CoinSelectionOption
// -----------------------------------------------------------------------------
CoinSelectionOption::CoinSelectionOption()
    : effective_fee_baserate_(FeeCalculator::kRelayMinimumFee),
      long_term_fee_baserate_(kDefaultLongTermFeeRate) {
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

uint64_t CoinSelectionOption::GetLongTermFeeBaserate() const {
  return long_term_fee_baserate_;
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

void CoinSelectionOption::SetEffectiveFeeBaserate(double baserate) {
  effective_fee_baserate_ = static_cast<uint64_t>(floor(baserate * 1000));
}

void CoinSelectionOption::SetLongTermFeeBaserate(double baserate) {
  long_term_fee_baserate_ = static_cast<uint64_t>(floor(baserate * 1000));
}

void CoinSelectionOption::SetTxNoInputsSize(size_t size) {
  tx_noinputs_size_ = size;
}

void CoinSelectionOption::InitializeTxSize(const TransactionController& tx) {
  tx_noinputs_size_ = tx.GetSizeIgnoreTxIn();
  change_output_size_ = 22 + 1 + 8;  // p2wpkh
  uint32_t witness_size = 0;
  uint32_t total_size = TxIn::EstimateTxInSize(
      AddressType::kP2wpkhAddress, Script(), &witness_size);
  change_spend_size_ = AbstractTransaction::GetVsizeFromSize(
      (total_size - witness_size), witness_size);
}

#ifndef CFD_DISABLE_ELEMENTS
ConfidentialAssetId CoinSelectionOption::GetFeeAsset() const {
  return fee_asset_;
}

void CoinSelectionOption::SetFeeAsset(const ConfidentialAssetId& asset) {
  fee_asset_ = asset;
}

void CoinSelectionOption::InitializeConfidentialTxSize(
    const ConfidentialTransactionController& tx) {
  uint32_t size;
  uint32_t witness_size = 0;

  size = tx.GetSizeIgnoreTxIn(true, &witness_size);
  tx_noinputs_size_ = AbstractTransaction::GetVsizeFromSize(
      (size - witness_size), witness_size);

  // wpkh想定
  Script wpkh_script("0014ffffffffffffffffffffffffffffffffffffffff");
  ConfidentialTxOut ctxout(
      wpkh_script, ConfidentialAssetId(), ConfidentialValue());
  ConfidentialTxOutReference txout(ctxout);
  witness_size = 0;
  size = txout.GetSerializeSize(true, &witness_size);
  change_output_size_ = AbstractTransaction::GetVsizeFromSize(
      (size - witness_size), witness_size);

  witness_size = 0;
  size = ConfidentialTxIn::EstimateTxInSize(
      AddressType::kP2wpkhAddress, Script(), 0, Script(), false, false,
      &witness_size);
  change_spend_size_ = AbstractTransaction::GetVsizeFromSize(
      (size - witness_size), witness_size);
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
    Amount* select_value, Amount* fee_value) {
  // for btc default(DUST_RELAY_TX_FEE(3000)) -> DEFAULT_DISCARD_FEE(10000)
  static constexpr const uint64_t kDustRelayTxFee = 10000;
  if (select_value) {
    *select_value = Amount::CreateBySatoshiAmount(0);
  } else {
    cfd::core::logger::info(
        CFD_LOG_SOURCE, "select_value=null. filter.asset={}",
        filter.target_asset.GetHex());
  }
  if (fee_value) {
    *fee_value = Amount::CreateBySatoshiAmount(0);
  }
  // Copy the list to change the calculation area.
  std::vector<Utxo> work_utxos = utxos;

  bool bnb_used = false;
  std::vector<Utxo*> utxo_pool;
  if (use_bnb_ && option_params.IsUseBnB()) {
    // Get long term estimate
    FeeCalculator discard_fee(kDustRelayTxFee);
    FeeCalculator effective_fee(option_params.GetEffectiveFeeBaserate());
    FeeCalculator long_term_fee(option_params.GetLongTermFeeBaserate());
    // FeeCalculation feeCalc;
    // CCoinControl temp;
    // temp.m_confirm_target = 1008;
    // CFeeRate long_term_feerate = GetMinimumFeeRate(*this, temp, &feeCalc);

    // Calculate cost of change
    // CAmount cost_of_change = GetDiscardRate(*this).GetFee(
    //   coin_selection_params.change_spend_size) +
    //   coin_selection_params.effective_fee.GetFee(
    //     coin_selection_params.change_output_size);
    Amount cost_of_change =
        discard_fee.GetFee(option_params.GetChangeSpendSize()) +
        effective_fee.GetFee(option_params.GetChangeOutputSize());

    // NOLINT Filter by the min conf specs and add to utxo_pool and calculate effective value
    for (auto& utxo : work_utxos) {
      // if (!group.EligibleForSpending(eligibility_filter)) continue;

      utxo.fee = 0;
      utxo.long_term_fee = 0;
      utxo.effective_value = 0;

      uint64_t fee = effective_fee.GetFee(utxo).GetSatoshiValue();
      // Only include outputs that are positive effective value (i.e. not dust)
      if (utxo.amount > fee) {
        uint64_t effective_value = utxo.amount - fee;
        utxo.fee = fee;
        utxo.long_term_fee = long_term_fee.GetFee(utxo).GetSatoshiValue();
#if 0
        std::vector<uint8_t> txid_byte(sizeof(utxo.txid));
        memcpy(txid_byte.data(), utxo.txid, txid_byte.size());
        info(
            CFD_LOG_SOURCE, "utxo({},{}) size={}/{} amount={}/{}/{}",
            Txid(txid_byte).GetHex(), utxo.vout, utxo.uscript_size_max,
            utxo.witness_size_max, utxo.amount, utxo.fee, utxo.long_term_fee);
#endif
        if (utxo.long_term_fee > utxo.fee) {
          utxo.long_term_fee = utxo.fee;  // TODO(k-matsuzawa): 後で見直し
        }
        utxo.effective_value = effective_value;
        utxo_pool.push_back(&utxo);
      }
    }
    // Calculate the fees for things that aren't inputs
    Amount not_input_fees =
        effective_fee.GetFee(option_params.GetTxNoInputsSize());
    std::vector<Utxo> result = SelectCoinsBnB(
        target_value, utxo_pool, cost_of_change, not_input_fees, select_value);
    if (!result.empty()) {
      if (select_value && fee_value &&
          (select_value->GetSatoshiValue() != 0)) {
        *fee_value = *select_value - target_value;
      }
      return result;
    }
    bnb_used = true;
    // SelectCoinsBnB fail, go to KnapsackSolver.
  }

  // Filter by the min conf specs and add to utxo_pool
  // TODO(k-matsuzawa): 現状はフィルタリングしていないためUtxo一覧の再作成不要
  // for (const OutputGroup& group : groups) {
  //   if (!group.EligibleForSpending(eligibility_filter)) continue;
  //   utxo_pool.push_back(group);
  // }
  if (!bnb_used) {
    for (auto& utxo : work_utxos) {
      if (utxo.effective_value == 0) {
        utxo.effective_value = utxo.amount;
      }
      utxo_pool.push_back(&utxo);
    }
  }
  std::vector<Utxo> result =
      KnapsackSolver(target_value, utxo_pool, select_value);
  if (select_value && fee_value && (select_value->GetSatoshiValue() != 0)) {
    *fee_value = *select_value - target_value;
  }
  return result;
}

std::vector<Utxo> CoinSelection::SelectCoinsBnB(
    const Amount& target_value, const std::vector<Utxo*>& utxos,
    const Amount& cost_of_change, const Amount& not_input_fees,
    Amount* select_value) {
  if (select_value == nullptr) {
    warn(CFD_LOG_SOURCE, "Outparameter(select_value) is nullptr.");
    throw CfdException(
        CfdError::kCfdIllegalArgumentError,
        "Failed to select coin. Outparameter is nullptr.");
  }
  info(
      CFD_LOG_SOURCE,
      "SelectCoinsBnB start. cost_of_change={}, not_input_fees={}",
      cost_of_change.GetSatoshiValue(), not_input_fees.GetSatoshiValue());

  std::vector<Utxo> results;
  Amount curr_value = Amount::CreateBySatoshiAmount(0);

  std::vector<bool> curr_selection;
  curr_selection.reserve(utxos.size());
  Amount actual_target = not_input_fees + target_value;

  // Calculate curr_available_value
  Amount curr_available_value = Amount::CreateBySatoshiAmount(0);
  for (const Utxo* utxo : utxos) {
    // Assert that this utxo is not negative. It should never be negative,
    //  effective value calculation should have removed it
    // assert(utxo->effective_value > 0);
    if (utxo->effective_value == 0) {
      warn(
          CFD_LOG_SOURCE,
          "Failed to SelectCoinsBnB. effective_value is 0."
          ": effective_value={}",
          utxo->effective_value);
      throw CfdException(
          CfdError::kCfdIllegalStateError,
          "Failed to select coin. effective amount is 0.");
    }
    curr_available_value += utxo->effective_value;
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

  // Sort the utxos
  std::vector<Utxo*> p_utxos = utxos;
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
      curr_waste -= (curr_value - actual_target);
      //NOLINT Remove the excess value as we will be selecting different coins now
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
  if (!best_selection.empty()) {
    // Set output set
    *select_value = Amount::CreateBySatoshiAmount(0);
    for (size_t i = 0; i < best_selection.size(); ++i) {
      if (best_selection.at(i)) {
        results.push_back(*(p_utxos.at(i)));
        *select_value += static_cast<int64_t>(p_utxos.at(i)->amount);
      }
    }
  }

  info(CFD_LOG_SOURCE, "SelectCoinsBnB end. results={}", results.size());
  return results;
}

std::vector<Utxo> CoinSelection::KnapsackSolver(
    const Amount& target_value, const std::vector<Utxo*>& utxos,
    Amount* select_value) {
  std::vector<Utxo> ret_utxos;

  if (select_value == nullptr) {
    warn(CFD_LOG_SOURCE, "Outparameter(select_value) is nullptr.");
    throw CfdException(
        CfdError::kCfdIllegalArgumentError,
        "Failed to select coin. Outparameter is nullptr.");
  }
  info(CFD_LOG_SOURCE, "KnapsackSolver start.");

  // List of values less than target
  const Utxo* lowest_larger = nullptr;
  std::vector<const Utxo*> applicable_groups;
  uint64_t n_total = 0;
  uint64_t n_target = target_value.GetSatoshiValue();

  std::vector<uint32_t> indexes =
      RandomNumberUtil::GetRandomIndexes(static_cast<uint32_t>(utxos.size()));

  for (size_t index = 0; index < indexes.size(); ++index) {
    if (utxos[index]->amount == n_target) {
      ret_utxos.push_back(*utxos[index]);
      *select_value = Amount::CreateBySatoshiAmount(utxos[index]->amount);
      info(CFD_LOG_SOURCE, "KnapsackSolver end. results={}", ret_utxos.size());
      return ret_utxos;

    } else if (utxos[index]->amount < n_target + kMinChange) {
      applicable_groups.push_back(utxos[index]);
      n_total += utxos[index]->amount;

    } else if (
        lowest_larger == nullptr ||
        utxos[index]->amount < lowest_larger->amount) {
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
    info(CFD_LOG_SOURCE, "KnapsackSolver end. results={}", ret_utxos.size());
    return ret_utxos;
  }

  if (n_total < n_target) {
    if (lowest_larger == nullptr) {
      warn(
          CFD_LOG_SOURCE, "insufficient funds. total:{} target:{}", n_total,
          n_target);
      throw CfdException(
          CfdError::kCfdIllegalStateError, "insufficient funds.");
    }

    ret_utxos.push_back(*lowest_larger);
    *select_value = Amount::CreateBySatoshiAmount(lowest_larger->amount);
    info(CFD_LOG_SOURCE, "KnapsackSolver end. results={}", ret_utxos.size());
    return ret_utxos;
  }

  std::sort(
      applicable_groups.begin(), applicable_groups.end(),
      [](const Utxo* a, const Utxo* b) {
        return a->effective_value > b->effective_value;
      });
  std::vector<char> vf_best;
  uint64_t n_best;

  randomize_cache_.clear();
  ApproximateBestSubset(
      applicable_groups, n_total, n_target, &vf_best, &n_best,
      kApproximateBestSubsetIterations);
  if (n_best != n_target && n_total >= n_target + kMinChange) {
    ApproximateBestSubset(
        applicable_groups, n_total, (n_target + kMinChange), &vf_best, &n_best,
        kApproximateBestSubsetIterations);
  }

  // NOLINT If we have a bigger coin and (either the stochastic approximation didn't find a good solution,
  // NOLINT                                or the next bigger coin is closer), return the bigger coin
  if (lowest_larger != nullptr &&
      ((n_best != n_target && n_best < n_target + kMinChange) ||
       lowest_larger->amount <= n_best)) {
    ret_utxos.push_back(*lowest_larger);
    *select_value = Amount::CreateBySatoshiAmount(lowest_larger->amount);

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
  info(CFD_LOG_SOURCE, "KnapsackSolver end. results={}", ret_utxos.size());
  return ret_utxos;
}

void CoinSelection::ApproximateBestSubset(
    const std::vector<const Utxo*>& utxos, uint64_t n_total_value,
    uint64_t n_target_value, std::vector<char>* vf_best, uint64_t* n_best,
    int iterations) {
  if (vf_best == nullptr || n_best == nullptr) {
    warn(CFD_LOG_SOURCE, "Outparameter(select_value) is nullptr.");
    throw CfdException(
        CfdError::kCfdIllegalArgumentError,
        "Failed to select coin. Outparameter is nullptr.");
  }

  std::vector<char> vf_includes;
  vf_best->assign(utxos.size(), true);
  *n_best = n_total_value;

  for (int n_rep = 0; n_rep < iterations && *n_best != n_target_value;
       n_rep++) {
    vf_includes.assign(utxos.size(), false);
    uint64_t n_total = 0;
    bool is_reached_target = false;
    for (int n_pass = 0; n_pass < 2 && !is_reached_target; n_pass++) {
      for (unsigned int i = 0; i < utxos.size(); i++) {
        // The solver here uses a randomized algorithm,
        // the randomness serves no real security purpose but is just
        // needed to prevent degenerate behavior and it is important
        // that the rng is fast. We do not use a constant random sequence,
        // because there may be some privacy improvement by making
        // the selection random.
        bool rand_bool = !vf_includes[i];
        if (n_pass == 0) {
          rand_bool = RandomNumberUtil::GetRandomBool(&randomize_cache_);
        }
        if (rand_bool) {
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

void CoinSelection::ConvertToUtxo(
    const Txid& txid, uint32_t vout, const std::string& output_descriptor,
    const Amount& amount, const std::string& asset, const void* binary_data,
    Utxo* utxo) {
  static constexpr const uint16_t kScriptSize = 50;
  if (utxo == nullptr) {
    warn(CFD_LOG_SOURCE, "utxo is nullptr.");
    throw CfdException(
        CfdError::kCfdIllegalArgumentError,
        "Failed to convert utxo. utxo is nullptr.");
  }
  memset(utxo, 0, sizeof(*utxo));
  ByteData txid_data = txid.GetData();
  if (!txid_data.Empty()) {
    memcpy(utxo->txid, txid_data.GetBytes().data(), sizeof(utxo->txid));
  }
  utxo->vout = vout;

  // TODO(k-matsuzawa): descriptor解析は暫定対処。本対応が必要。

  uint32_t minimum_txin = static_cast<uint32_t>(TxIn::kMinimumTxInSize);
  uint32_t witness_size = 0;
  if (output_descriptor.find("wpkh(") == 0) {
    utxo->address_type = AddressType::kP2wpkhAddress;
    TxIn::EstimateTxInSize(
        AddressType::kP2wpkhAddress, Script(), &witness_size);
    utxo->witness_size_max = static_cast<uint16_t>(witness_size);
  } else if (output_descriptor.find("wsh(") == 0) {
    utxo->address_type = AddressType::kP2wshAddress;
    TxIn::EstimateTxInSize(
        AddressType::kP2wshAddress, Script(), &witness_size);
    utxo->witness_size_max = static_cast<uint16_t>(witness_size);
    utxo->witness_size_max += kScriptSize;
  } else if (output_descriptor.find("sh(") == 0) {
    if (output_descriptor.find("sh(wpkh(") == 0) {
      utxo->address_type = AddressType::kP2shP2wpkhAddress;
      utxo->uscript_size_max = 22;
      TxIn::EstimateTxInSize(
          AddressType::kP2wpkhAddress, Script(), &witness_size);
      utxo->witness_size_max = static_cast<uint16_t>(witness_size);
    } else if (output_descriptor.find("sh(wsh(") == 0) {
      utxo->address_type = AddressType::kP2shP2wshAddress;
      utxo->uscript_size_max = 34;
      TxIn::EstimateTxInSize(
          AddressType::kP2wshAddress, Script(), &witness_size);
      utxo->witness_size_max = static_cast<uint16_t>(witness_size);
      utxo->witness_size_max += kScriptSize;
    } else {
      utxo->address_type = AddressType::kP2shAddress;
      utxo->uscript_size_max = kScriptSize;
      utxo->uscript_size_max +=
          TxIn::EstimateTxInSize(AddressType::kP2shAddress, Script()) -
          minimum_txin;
    }
  } else if (output_descriptor.find("pkh(") == 0) {
    utxo->address_type = AddressType::kP2pkhAddress;
    utxo->uscript_size_max =
        TxIn::EstimateTxInSize(AddressType::kP2pkhAddress, Script()) -
        minimum_txin;
  } else {
    // unknown type?
  }
#if 0
  const std::vector<uint8_t>& scrpt = locking_script.GetData().GetBytes();
  if (scrpt.size() < sizeof(utxo->locking_script)) {
    memcpy(utxo->locking_script, scrpt.data(), sizeof(utxo->locking_script));
    utxo->script_length = static_cast<uint16_t>(scrpt.size());
  }
#endif

  utxo->amount = amount.GetSatoshiValue();
  memcpy(&utxo->binary_data, &binary_data, sizeof(void*));
  utxo->effective_value = utxo->amount;

#ifndef CFD_DISABLE_ELEMENTS
  if (!asset.empty()) {
    ConfidentialAssetId asset_data(asset);
    utxo->blinded = asset_data.HasBlinding();
    memcpy(
        utxo->asset, asset_data.GetData().GetBytes().data(),
        sizeof(utxo->asset));
  }
#endif  // CFD_DISABLE_ELEMENTS
}

void CoinSelection::ConvertToUtxo(
    uint64_t block_height, const BlockHash& block_hash, const Txid& txid,
    uint32_t vout, const Script& locking_script,
    const std::string& output_descriptor, const Amount& amount,
    const void* binary_data, Utxo* utxo) {
  static constexpr const uint16_t kScriptSize = 50;
  if (utxo == nullptr) {
    warn(CFD_LOG_SOURCE, "utxo is nullptr.");
    throw CfdException(
        CfdError::kCfdIllegalArgumentError,
        "Failed to convert utxo. utxo is nullptr.");
  }
  memset(utxo, 0, sizeof(*utxo));

  utxo->block_height = block_height;
  if (!block_hash.GetData().Empty()) {
    memcpy(
        utxo->block_hash, block_hash.GetData().GetBytes().data(),
        sizeof(utxo->block_hash));
  }
  ByteData txid_data = txid.GetData();
  if (!txid_data.Empty()) {
    memcpy(utxo->txid, txid_data.GetBytes().data(), sizeof(utxo->txid));
  }
  utxo->vout = vout;
  const std::vector<uint8_t>& script = locking_script.GetData().GetBytes();
  if (script.size() < sizeof(utxo->locking_script)) {
    memcpy(utxo->locking_script, script.data(), script.size());
    utxo->script_length = static_cast<uint16_t>(script.size());
  }

  uint32_t minimum_txin = static_cast<uint32_t>(TxIn::kMinimumTxInSize);
  uint32_t witness_size = 0;
  if (locking_script.IsP2pkhScript()) {
    utxo->address_type = AddressType::kP2pkhAddress;
    utxo->uscript_size_max =
        TxIn::EstimateTxInSize(AddressType::kP2pkhAddress, Script()) -
        minimum_txin;
  } else if (locking_script.IsP2shScript()) {
    utxo->address_type = AddressType::kP2shAddress;
    utxo->uscript_size_max = kScriptSize;
    utxo->uscript_size_max +=
        TxIn::EstimateTxInSize(AddressType::kP2shAddress, Script()) -
        minimum_txin;
    if (output_descriptor.find("sh(wpkh(") == 0) {
      utxo->address_type = AddressType::kP2shP2wpkhAddress;
      utxo->uscript_size_max = 22;
      TxIn::EstimateTxInSize(
          AddressType::kP2wpkhAddress, Script(), &witness_size);
      utxo->witness_size_max = static_cast<uint16_t>(witness_size);
    } else if (output_descriptor.find("sh(wsh(") == 0) {
      utxo->address_type = AddressType::kP2shP2wshAddress;
      utxo->uscript_size_max = 34;
      TxIn::EstimateTxInSize(
          AddressType::kP2wshAddress, Script(), &witness_size);
      utxo->witness_size_max = static_cast<uint16_t>(witness_size);
      utxo->witness_size_max += kScriptSize;
    }
  } else if (locking_script.IsP2wpkhScript()) {
    utxo->address_type = AddressType::kP2wpkhAddress;
    TxIn::EstimateTxInSize(
        AddressType::kP2wpkhAddress, Script(), &witness_size);
    utxo->witness_size_max = static_cast<uint16_t>(witness_size);
  } else if (locking_script.IsP2wshScript()) {
    utxo->address_type = AddressType::kP2wshAddress;
    TxIn::EstimateTxInSize(
        AddressType::kP2wshAddress, Script(), &witness_size);
    utxo->witness_size_max = static_cast<uint16_t>(witness_size);
    utxo->witness_size_max += kScriptSize;
  }
  // TODO(k-matsuzawa): 後でOutputDescriptor対応する
  utxo->amount = amount.GetSatoshiValue();
  memcpy(&utxo->binary_data, &binary_data, sizeof(void*));
  utxo->effective_value = utxo->amount;
}

#ifndef CFD_DISABLE_ELEMENTS
void CoinSelection::ConvertToUtxo(
    uint64_t block_height, const BlockHash& block_hash, const Txid& txid,
    uint32_t vout, const Script& locking_script,
    const std::string& output_descriptor, const Amount& amount,
    const ConfidentialAssetId& asset, const void* binary_data, Utxo* utxo) {
  ConvertToUtxo(
      block_height, block_hash, txid, vout, locking_script, output_descriptor,
      amount, binary_data, utxo);
  utxo->blinded = asset.HasBlinding();
  memcpy(utxo->asset, asset.GetData().GetBytes().data(), sizeof(utxo->asset));
}
#endif  // CFD_DISABLE_ELEMENTS

}  // namespace cfd
