// Copyright 2019 CryptoGarage
/**
 * @file cfd_utxo.h
 *
 * @brief UTXO操作の関連クラス定義
 */
#ifndef CFD_INCLUDE_CFD_CFD_UTXO_H_
#define CFD_INCLUDE_CFD_CFD_UTXO_H_

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include "cfd/cfd_common.h"
#include "cfdcore/cfdcore_address.h"
#include "cfdcore/cfdcore_amount.h"
#include "cfdcore/cfdcore_coin.h"
#include "cfdcore/cfdcore_elements_transaction.h"
#include "cfdcore/cfdcore_script.h"

namespace cfd {

using cfd::core::Address;
using cfd::core::Amount;
using cfd::core::BlockHash;
using cfd::core::Script;
using cfd::core::Txid;
#ifndef CFD_DISABLE_ELEMENTS
using cfd::core::ConfidentialAssetId;
#endif  // CFD_DISABLE_ELEMENTS

/**
 * @brief 最小のデータのみを保持するUTXO構造体。
 * @details witness_size_max, uscript_size_max, address_type
 *  は専用APIで算出する。
 */
struct Utxo {
  uint64_t block_height;     //!< blick高
  uint8_t block_hash[32];    //!< block hash
  uint8_t txid[32];          //!< txid
  uint32_t vout;             //!< vout
  // uint32_t script_length;    //!< script length
  uint16_t witness_size_max; //!< witness stack size maximum
  uint16_t uscript_size_max; //!< unlocking script size maximum
  uint64_t amount;           //!< amount
  uint16_t address_type;     //!< address type (cfd::core::AddressType)
#ifndef CFD_DISABLE_ELEMENTS
  bool blinded;       //!< has blind
  uint8_t asset[33];  //!< asset
#endif                // CFD_DISABLE_ELEMENTS
#if 0
  int32_t status;           //!< utxo status (reserved)
  // elements
  uint8_t confidential_key[33];      //!< Confidential key
#endif  // if 0
  // calculate
  uint64_t effective_value;  //!< amountからfeeを除外した有効額
  uint64_t fee;              //!< fee
  uint64_t long_term_fee;    //!< 長期間後のfee
};

/**
 * @brief UTXOのフィルタリング条件を指定する。
 */
struct UtxoFilter {
#ifndef CFD_DISABLE_ELEMENTS
  std::string include_asset;  //!< 利用するasset
#endif                        // CFD_DISABLE_ELEMENTS
  uint32_t reserved;          //!< 予約領域
};

/**
 * @brief CoinSelectionのオプション情報を保持するクラス
 */
class CFD_EXPORT CoinSelectionOption {
 public:
  /**
   * @brief コンストラクタ
   */
  CoinSelectionOption();

  /**
   * @brief BnB使用フラグを取得します.
   * @retval true BnB使用
   * @retval false BnB未使用
   */
  bool IsUseBnB() const;
  /**
   * @brief 出力変更サイズを取得します.
   * @return 出力変更サイズ
   */
  size_t GetChangeOutputSize() const;
  /**
   * @brief 出力変更サイズを取得します.
   * @return 出力変更サイズ
   */
  size_t GetChangeSpendSize() const;
  /**
   * @brief 効果的なfeeのbase rateを取得します.
   * @return 効果的なfeeのbase rate
   */
  uint64_t GetEffectiveFeeBaseRate() const;
  /**
   * @brief 出力変更サイズを取得します.
   * @return 出力変更サイズ
   */
  size_t GetTxNoInputsSize() const;

  /**
   * @brief BnB 使用フラグを設定します.
   * @param[in] use_bnb   BnB 使用フラグ
   */
  void SetUseBnB(bool use_bnb);
  /**
   * @brief 出力変更サイズを設定します.
   * @param[in] size    サイズ
   */
  void SetChangeOutputSize(size_t size);
  /**
   * @brief 受入変更サイズを設定します.
   * @param[in] size    サイズ
   */
  void SetChangeSpendSize(size_t size);
  /**
   * @brief 効果的なfeeのbase rateを設定します.
   * @param[in] base_rate   fee base rate
   */
  void SetEffectiveFeeBaseRate(uint64_t base_rate);
  /**
   * @brief tx合計サイズのうちTxIn分のサイズを差し引いたサイズを設定する。
   * @param[in] size    ignore txin size.
   * @see cfd::TransactionController::GetSizeIgnoreTxIn()
   * @see cfd::ConfidentialTransactionController::GetSizeIgnoreTxIn()
   */
  void SetTxNoInputsSize(size_t size);

#ifndef CFD_DISABLE_ELEMENTS
  /**
   * @brief feeに使用するassetを取得します.
   * @return feeに使用するasset
   */
  ConfidentialAssetId GetFeeAsset() const;
  /**
   * @brief feeに使用するassetを設定します.
   * @param[in] asset   asset
   */
  void SetFeeAsset(const ConfidentialAssetId& asset);
#endif  // CFD_DISABLE_ELEMENTS

 private:
  bool use_bnb_ = true;                  //!< BnB 使用フラグ
  size_t change_output_size_ = 0;        //!< 出力変更サイズ
  size_t change_spend_size_ = 0;         //!< 受入変更サイズ
  uint64_t effective_fee_baserate_ = 0;  //!< feeのbaserate
  /**
   * @brief txのTxIn除外時のサイズ.
   * @details elementsなどの考慮を算出時点で行うこと。
   */
  size_t tx_noinputs_size_ = 0;
#ifndef CFD_DISABLE_ELEMENTS
  ConfidentialAssetId fee_asset_;  //!< feeとして利用するasset
#endif                             // CFD_DISABLE_ELEMENTS
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
  explicit CoinSelection(bool use_bnb);

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
  std::vector<Utxo> SelectCoinsMinConf(
      const Amount& target_value, const std::vector<Utxo>& utxos,
      const UtxoFilter& filter, const CoinSelectionOption& option_params,
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
  std::vector<Utxo> SelectCoinsBnB(
      const Amount& target_value, const std::vector<Utxo>& utxos,
      const Amount& cost_of_change, const Amount& not_input_fees,
      Amount* select_value) const;

  /**
   * @brief CoinSelection(KnapsackSolver)を実施する。
   * @param[in] target_value    収集額
   * @param[in] utxos           検索対象UTXO一覧
   * @param[out] select_value   UTXO収集成功時、合計収集額
   * @return UTXO一覧。空の場合はエラー終了。
   */
  std::vector<Utxo> KnapsackSolver(
      const Amount& target_value, const std::vector<Utxo>& utxos,
      Amount* select_value) const;

 private:
  bool use_bnb_;  //!< BnB 利用フラグ
};

}  // namespace cfd

#endif  // CFD_INCLUDE_CFD_CFD_UTXO_H_
