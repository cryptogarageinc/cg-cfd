// Copyright 2019 CryptoGarage
/**
 * @file cfd_utxo.h
 *
 * @brief UTXO操作の関連クラス定義
 */
#ifndef CFD_INCLUDE_CFD_CFD_UTXO_H_
#define CFD_INCLUDE_CFD_CFD_UTXO_H_

#include <string>
#include <vector>

#include "cfd/cfd_common.h"
#include "cfdcore/cfdcore_address.h"
#include "cfdcore/cfdcore_amount.h"
#include "cfdcore/cfdcore_coin.h"
#include "cfdcore/cfdcore_elements_address.h"
#include "cfdcore/cfdcore_elements_transaction.h"
#include "cfdcore/cfdcore_script.h"

namespace cfd {

using cfd::core::Amount;
using cfd::core::BlockHash;
using cfd::core::Txid;
using cfd::core::Script;
using cfd::core::Address;
using cfd::core::BlindFactor;
using cfd::core::ConfidentialAddress;
using cfd::core::ConfidentialAssetId;

/**
 * @brief UTXO構造体
 */
struct Utxo {
  uint64_t block_height;    //<! blick高
  std::string block_hash;   //<! block hash
  std::string txid;         //<! txid
  uint32_t vout;            //<! vout
  std::string script;       //<! script
  std::string address;      //<! address
  std::string descriptor;   //<! output descriptor
  uint64_t amount;          //<! amount
#if 0
  int32_t status;           //<! utxo status (reserved)
#ifndef CFD_DISABLE_ELEMENTS
  // elements
  ConfidentialAddress confidential_address;   //!< Confidential address
  ConfidentialAssetId asset;                  //<! asset
  BlindFactor asset_blind_factor;             //<! asset blind factor
  BlindFactor amount_blind_factor;            //<! blind vactor
#endif  // CFD_DISABLE_ELEMENTS
#endif  // if 0
  // calculate
  uint64_t effective_value;   //<! amountからfeeを除外した有効額
  uint64_t fee;               //<! fee
  uint64_t long_term_fee;     //<! 長期間後のfee
}

/**
 * @brief UTXOのフィルタリング条件を指定する。
 */
struct UtxoFilter {
#ifndef CFD_DISABLE_ELEMENTS
  ConfidentialAssetId include_asset;    //!< 利用するasset
#endif  // CFD_DISABLE_ELEMENTS
  uint32_t reserved;
};


struct CoinSelectionOption
{
 public:
  bool use_bnb = true;            //!< 
  size_t change_output_size = 0;  //!< 
  size_t change_spend_size = 0;   //!< 
  uint64_t effective_fee_baserate = 0;  //!< feeのbaserate
  /**
   * @brief txのTxIn除外時のサイズ.
   * @details elementsなどの考慮を算出時点で行うこと。
   */
  size_t tx_noinputs_size = 0;
#ifndef CFD_DISABLE_ELEMENTS
  ConfidentialAssetId fee_asset;  //!< feeとして利用するasset
#endif  // CFD_DISABLE_ELEMENTS

  /**
   * @brief コンストラクタ定義
   */
  CoinSelectionOption();
};

/**
 * @brief CoinSelection計算を行うクラス
 */
class CFD_EXPORT CoinSelection {
 public:
  /**
   * @brief コンストラクタ
   */
  CoinSelection();

  /**
   * @brief コンストラクタ
   * @param[in] use_bnb   
   */
  CoinSelection(bool use_bnb);

  /**
   * @brief 最小のCoinを選択する。
   * @param[in] target_value    収集額
   * @param[in] utxos           検索対象UTXO一覧
   * @param[in] filter          UTXO収集フィルタ情報
   * @param[in] option_params   オプション情報
   * @param[out] select_value   UTXO収集成功時、合計収集額
   * @param[out] fee_value      UTXO収集成功時、fee金額
   * @return UTXO一覧。空の場合はエラー終了。
   */
  std::vector<Utxo> SelectCoinsMinConf(const Amount& target_value,
    const std::vector<Utxo>& utxos, const UtxoFilter& filter,
    const CoinSelectionOption& option_params,
    Amount* select_value = nullptr, Amount* fee_value = nullptr) const;

  /**
   * @brief CoinSelection(BnB)を実施する。
   * @param[in] target_value    収集額
   * @param[in] utxos           検索対象UTXO一覧
   * @param[in] cost_of_change  コストの変更範囲。
   *              target_value+本値が収集上限値となる。
   * @param[in] not_input_fees  TxIn部を除いたfee額
   * @param[out] select_value   UTXO収集成功時、合計収集額
   * @return UTXO一覧。空の場合はエラー終了。
   */
  std::vector<Utxo> SelectCoinsBnB(const Amount& target_value, const std::vector<Utxo>& utxos,
    const Amount& cost_of_change, const Amount& not_input_fees,
    CAmount* select_value);

  /**
   * @brief CoinSelection(KnapsackSolver)を実施する。
   * @param[in] target_value    収集額
   * @param[in] utxos           検索対象UTXO一覧
   * @param[out] select_value   UTXO収集成功時、合計収集額
   * @return UTXO一覧。空の場合はエラー終了。
   */
  std::vector<Utxo> KnapsackSolver(const Amount& target_value, const std::vector<Utxo>& utxos,
    Amount* select_value);

 private:
  bool use_bnb_ = true;     //<! BnB 利用フラグ
};

}  // namespace cfd

#endif  // CFD_INCLUDE_CFD_CFD_UTXO_H_
