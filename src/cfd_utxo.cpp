// Copyright 2019 CryptoGarage
/**
 * @file cfd_utxo.cpp
 *
 * @brief UTXO操作の関連クラスの実装ファイル
 */
#include <string>
#include <vector>

#include "cfd/cfd_utxo.h"

#include "cfd/cfd_common.h"
#include "cfdcore/cfdcore_address.h"
#include "cfdcore/cfdcore_amount.h"
#include "cfdcore/cfdcore_coin.h"
#include "cfdcore/cfdcore_elements_transaction.h"
#include "cfdcore/cfdcore_script.h"

namespace cfd {

using cfd::core::ConfidentialAssetId;

// -----------------------------------------------------------------------------
// CoinSelectionOption
// -----------------------------------------------------------------------------
CoinSelectionOption() {
  // do nothing
}

bool CoinSelectionOption::IsUseBnB() const {
  return use_bnb_;
}

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

void CoinSelectionOption::SetUseBnB(bool use_bnb) {
  use_bnb_ = use_bnb;
}

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
