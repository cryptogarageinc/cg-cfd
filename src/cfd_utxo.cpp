// Copyright 2019 CryptoGarage
/**
 * @file cfd_utxo.cpp
 *
 * @brief UTXO操作の関連クラスの実装ファイル
 */
#include <algorithm>
#include <string>
#include <vector>

#include "cfd/cfd_utxo.h"

#include "cfd/cfd_common.h"
#include "cfdcore/cfdcore_address.h"
#include "cfdcore/cfdcore_amount.h"
#include "cfdcore/cfdcore_coin.h"
#include "cfdcore/cfdcore_elements_transaction.h"
#include "cfdcore/cfdcore_exception.h"
#include "cfdcore/cfdcore_logger.h"
#include "cfdcore/cfdcore_script.h"
#include "cfdcore/cfdcore_util.h"

namespace cfd {

using cfd::core::CfdError;
using cfd::core::CfdException;
using cfd::core::ConfidentialAssetId;
using cfd::core::RandomNumberUtil;
using cfd::core::logger::warn;

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

uint64_t CoinSelectionOption::GetEffectiveFeeBaseRate() const {
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

void CoinSelectionOption::SetEffectiveFeeBaseRate(uint64_t base_rate) {
  effective_fee_baserate_ = base_rate;
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
  // FIXME
  return std::vector<Utxo>();
}

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
    const std::vector<const Utxo*>& utxos, int64_t n_total_value,
    int64_t n_target_value, std::vector<char>& vf_best, int64_t* n_best,
    int iterations = 1000) {
  std::vector<char> vf_includes;
  vf_best.assign(utxos.size(), true);
  *n_best = n_total_value;

  for (int n_rep = 0; n_rep < iterations && *n_best != n_target_value;
       n_rep++) {
    vf_includes.assign(utxos.size(), false);
    int64_t n_total = 0;
    bool is_reached_target = false;
    for (int n_pass = 0; n_pass < 2 && !is_reached_target; n_pass++) {
      for (unsigned int i = 0; i < utxos.size(); i++) {
        //The solver here uses a randomized algorithm,
        //the randomness serves no real security purpose but is just
        //needed to prevent degenerate behavior and it is important
        //that the rng is fast. We do not use a constant random sequence,
        //because there may be some privacy improvement by making
        //the selection random.
        bool rand_bool = RandomNumberUtil::GetRandomBool();
        if (n_pass == 0 ? rand_bool : !vf_includes[i]) {
          n_total += utxos[i]->amount;
          vf_includes[i] = true;
          if (n_total >= n_target_value) {
            is_reached_target = true;
            if (n_total < *n_best) {
              *n_best = n_total;
              vf_best = vf_includes;
            }
            n_total -= utxos[i]->amount;
            vf_includes[i] = false;
          }
        }
      }
    }
  }
}

std::vector<Utxo> CoinSelection::KnapsackSolver(
    const Amount& target_value, const std::vector<Utxo>& utxos,
    Amount* select_value) const {
  std::vector<Utxo> ret_utxos;

  // List of values less than target
  Utxo lowest_larger;
  std::vector<const Utxo*> applicable_groups;
  int64_t n_total = 0;
  int64_t n_target = target_value.GetSatoshiValue();

  // UTXOをシャッフル
  std::vector<uint32_t> indexes =
      RandomNumberUtil::GetRandomIndexes(utxos.size());

  for (size_t index = 0; index < indexes.size(); ++index) {
    if (utxos[index].amount == n_target) {
      // ジャストのUTXOがあれば、その時点で終了
      ret_utxos.push_back(utxos[index]);
      *select_value = Amount::CreateBySatoshiAmount(utxos[index].amount);
      return ret_utxos;

    } else if (utxos[index].amount < n_target + kMinChange) {
      // 少額UTXO
      applicable_groups.push_back(&utxos[index]);
      n_total += utxos[index].amount;

    } else if (
        lowest_larger.amount == 0 ||
        utxos[index].amount < lowest_larger.amount) {
      // 収集値より大きい、最小のUTXO
      lowest_larger = utxos[index];
    }
  }

  if (n_total == n_target) {
    // ジャストのUTXOがあれば、その時点で終了
    int64_t ret_value = 0;
    for (const auto& utxo : applicable_groups) {
      ret_utxos.push_back(*utxo);
      ret_value += utxo->amount;
    }
    *select_value = Amount::CreateBySatoshiAmount(ret_value);
    return ret_utxos;
  }

  if (n_total < n_target) {
    if (lowest_larger.amount == 0) {
      // UTXOのtotalが収集額よりも少ない
      warn(
          CFD_LOG_SOURCE, "insufficient funds. total:{} target:{}", n_total,
          n_target);
      throw CfdException(
          CfdError::kCfdIllegalStateError, "insufficient funds.");
      return ret_utxos;
    }

    // 収集額より大きい、最小のUTXOを返却
    ret_utxos.push_back(lowest_larger);
    *select_value = Amount::CreateBySatoshiAmount(lowest_larger.amount);
    return ret_utxos;
  }

  // 対象のUTXOを降順ソート
  std::sort(
      applicable_groups.begin(), applicable_groups.end(),
      [](const Utxo*& a, const Utxo*& b) {
        return a->effective_value > b->effective_value;
      });
  std::vector<char> vf_best;
  int64_t n_best;

  // 少額UTXOをランダムに全探索し、最小の合計値を収集
  ApproximateBestSubset(
      applicable_groups, n_total, n_target, vf_best, &n_best);
  if (n_best != n_target && n_total >= n_target + kMinChange) {
    // ジャストじゃない場合＆収集金額より最小のおつり分を加算してもOKの場合、金額変更して再度収集
    ApproximateBestSubset(
        applicable_groups, n_total, (n_target + kMinChange), vf_best, &n_best);
  }

  // If we have a bigger coin and (either the stochastic approximation didn't find a good solution,
  //                                   or the next bigger coin is closer), return the bigger coin
  if (lowest_larger.amount != 0 &&
      ((n_best != n_target && n_best < n_target + kMinChange) ||
       lowest_larger.amount <= n_best)) {
    // 収集値より大きな最小UTXOがあり、ベスト値が不足しているorベスト額より最小UTXOが小さいとき、該当UTXOセットを加算
    ret_utxos.push_back(lowest_larger);
    *select_value = Amount::CreateBySatoshiAmount(lowest_larger.amount);

  } else {
    // 少額UTXOのベスト指定値を出力にセット
    int64_t ret_value = 0;
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

}  // namespace cfd
