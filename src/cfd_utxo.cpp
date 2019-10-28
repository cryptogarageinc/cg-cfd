// Copyright 2019 CryptoGarage
/**
 * @file cfd_utxo.cpp
 *
 * @brief UTXO操作の関連クラスの実装ファイル
 */

#include "cfd/cfd_utxo.h"

namespace cfd {

CoinSelection::CoinSelection() {
  // FIXME
}

CoinSelection::CoinSelection(bool use_bnb) {
  // FIXME
}

std::vector<Utxo> CoinSelection::SelectCoinsMinConf(const Amount& target_value,
    const std::vector<Utxo>& utxos, const UtxoFilter& filter,
    const CoinSelectionOption& option_params,
    Amount* select_value = nullptr, Amount* fee_value = nullptr) const {
  // FIXME
  return std::vector<Utxo>();
}

std::vector<Utxo> SelectCoinsBnB(const Amount& target_value, const std::vector<Utxo>& utxos,
    const Amount& cost_of_change, const Amount& not_input_fees,
    Amount* select_value) {
  // FIXME
  return std::vector<Utxo>();
}

std::vector<Utxo> KnapsackSolver(const Amount& target_value, const std::vector<Utxo>& utxos,
    Amount* select_value) {
  // FIXME
  return std::vector<Utxo>();
}

}  // namespace cfd


