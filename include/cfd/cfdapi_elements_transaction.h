// Copyright 2019 CryptoGarage
/**
 * @file cfdapi_elements_transaction.h
 *
 * @brief cfd-apiで利用するElements用Transaction操作のクラス定義
 *
 * JSON形式のAPIを提供する.
 */
#ifndef CFD_INCLUDE_CFD_CFDAPI_ELEMENTS_TRANSACTION_H_
#define CFD_INCLUDE_CFD_CFDAPI_ELEMENTS_TRANSACTION_H_
#ifndef CFD_DISABLE_ELEMENTS

#include <string>
#include <vector>

#include "cfd/cfd_common.h"
#include "cfd/cfd_elements_transaction.h"
#include "cfd/cfd_transaction_common.h"
#include "cfd/cfdapi_struct.h"
#include "cfdcore/cfdcore_coin.h"
#include "cfdcore/cfdcore_elements_transaction.h"
#include "cfdcore/cfdcore_key.h"
#include "cfdcore/cfdcore_transaction_common.h"
#include "cfdcore/cfdcore_util.h"

/**
 * @brief cfdapi名前空間
 */
namespace cfd {
namespace api {

using cfd::ConfidentialTransactionController;
using cfd::SignParameter;
using cfd::core::ByteData;
using cfd::core::ConfidentialTxIn;
using cfd::core::ConfidentialTxInReference;
using cfd::core::ConfidentialTxOut;
using cfd::core::ConfidentialValue;
using cfd::core::HashType;
using cfd::core::Privkey;
using cfd::core::Pubkey;
using cfd::core::Script;
using cfd::core::SigHashType;
using cfd::core::Txid;

/**
 * @brief Elements用Transaction関連の関数群クラス
 */
class CFD_EXPORT ElementsTransactionApi {
 public:
  /**
   * @brief constructor.
   */
  ElementsTransactionApi() {}

  /**
   * @brief Elements用のRaw Transactionを作成する.
   * @param[in] version     tx version
   * @param[in] locktime    lock time
   * @param[in] txins       tx input list
   * @param[in] txouts      tx output list
   * @param[in] txout_fee   tx output fee
   * @return Transaction
   */
  ConfidentialTransactionController CreateRawTransaction(
      uint32_t version, uint32_t locktime,
      const std::vector<ConfidentialTxIn>& txins,
      const std::vector<ConfidentialTxOut>& txouts,
      const ConfidentialTxOut& txout_fee) const;

  /**
   * @brief tx情報およびパラメータから、SigHashを作成する.
   * @param[in] tx_hex          tx hex string
   * @param[in] txin            target tx input
   * @param[in] pubkey          public key
   * @param[in] value           value (amount or commitment)
   * @param[in] hash_type       hash type
   * @param[in] sighash_type    sighash type
   * @return sighash
   */
  ByteData CreateSignatureHash(
      const std::string& tx_hex, const ConfidentialTxInReference& txin,
      const Pubkey& pubkey, const ConfidentialValue& value, HashType hash_type,
      const SigHashType& sighash_type) const;
  /**
   * @brief tx情報およびパラメータから、SigHashを作成する.
   * @param[in] tx_hex          tx hex string
   * @param[in] txin            target tx input
   * @param[in] redeem_script   redeem script
   * @param[in] value           value (amount or commitment)
   * @param[in] hash_type       hash type
   * @param[in] sighash_type    sighash type
   * @return sighash
   */
  ByteData CreateSignatureHash(
      const std::string& tx_hex, const ConfidentialTxInReference& txin,
      const Script& redeem_script, const ConfidentialValue& value,
      HashType hash_type, const SigHashType& sighash_type) const;
  /**
   * @brief tx情報およびパラメータから、SigHashを作成する.
   * @param[in] tx_hex          tx hex string
   * @param[in] txin            target tx input
   * @param[in] key_data        key data (pubkey or redeem script)
   * @param[in] value           value (amount or commitment)
   * @param[in] hash_type       hash type
   * @param[in] sighash_type    sighash type
   * @return sighash
   */
  ByteData CreateSignatureHash(
      const std::string& tx_hex, const ConfidentialTxInReference& txin,
      const ByteData& key_data, const ConfidentialValue& value,
      HashType hash_type, const SigHashType& sighash_type) const;

  /**
   * @brief Multisig署名情報を追加する.
   * @details 追加するsignatureの順序は、redeem
   * scriptのpubkeyとsign_list内のrelatedPubkeyで
   *   対応をとって自動的に整列される.
   * (relatedPubkeyが設定されていない場合は、relatedPubkeyが
   *   設定されているsignatureを追加した後にsignParamの順序でsignatureを追加)
   * @param[in] tx_hex          tx hex string
   * @param[in] txin            target tx input
   * @param[in] sign_list       value (amount or commitment)
   * @param[in] address_type    address type. (support is P2sh-P2wsh or P2wsh)
   * @param[in] witness_script  witness script
   * @param[in] redeem_script   redeem script
   * @param[in] clear_stack     clear stack data before add.
   * @return Transaction
   */
  ConfidentialTransactionController AddMultisigSign(
      const std::string& tx_hex, const ConfidentialTxInReference& txin,
      const std::vector<SignParameter>& sign_list, AddressType address_type,
      const Script& witness_script, const Script redeem_script = Script(),
      bool clear_stack = true);

  /*
   * @brief Issue用BlindingKeyを作成する.
   * @param[in] master_blinding_key master blindingKey
   * @param[in] txid                issuance utxo txid
   * @param[in] vout                issuance utxo vout
   * @return blinding key
   */
  // 別クラスに分ける。Struct系のAPIを ～StructApi というクラスにした方が良い
  // Privkey GetIssuanceBlindingKey(const Privkey& master_blinding_key,
  //     const Txid& txid, int32_t vout);
};

}  // namespace api
}  // namespace cfd

namespace cfd {
namespace js {
namespace api {

/**
 * @brief Elements用Transaction関連の関数群クラス
 */
class CFD_EXPORT ElementsTransactionStructApi {
 public:
  /**
   * @brief パラメータの情報を元に、Elements用のRaw Transactionを作成する.
   * @param[in] request Transactionを構築するパラメータの構造体
   * @return Transactionのhexデータを格納した構造体
   */
  static ElementsCreateRawTransactionResponseStruct CreateRawTransaction(
      const ElementsCreateRawTransactionRequestStruct& request);

  /**
   * @brief パラメータの情報を元に、Transactionをデコードして出力する.
   * @param[in] request Transactionとデコード用の情報を格納した構造体
   * @return Transactionの表示用データを格納した構造体
   */
  static ElementsDecodeRawTransactionResponseStruct DecodeRawTransaction(
      const ElementsDecodeRawTransactionRequestStruct& request);

  /**
   * @brief パラメータの情報を元に、WitnessStack数を出力する.
   * @param[in] request Transactionと対象TxIn情報を格納した構造体
   * @return WitnessStack数を格納した構造体
   */
  static GetWitnessStackNumResponseStruct GetWitnessStackNum(
      const GetWitnessStackNumRequestStruct& request);

  /**
   * @brief パラメータの情報を元に、署名情報を追加する.
   * @param[in] request Transactionと署名情報を格納した構造体
   * @return Transactionのhexデータを格納した構造体
   */
  static AddSignResponseStruct AddSign(const AddSignRequestStruct& request);

  /**
   * @brief パラメータの情報を元に、SegwitのMultisig署名情報を追加する.
   * @details 追加するsignatureの順序は、redeem
   * scriptのpubkeyとsignParam内のrelatedPubkeyで
   *   対応をとって自動的に整列される.
   * (relatedPubkeyが設定されていない場合は、relatedPubkeyが
   *   設定されているsignatureを追加した後にsignParamの順序でsignatureを追加)
   * @param[in] request TransactionとSegwitのMultisig署名情報を格納した構造体
   * @return Transactionのhexデータを格納した構造体
   */
  static AddMultisigSignResponseStruct AddMultisigSign(
      const AddMultisigSignRequestStruct& request);

  /**
   * @brief パラメータの情報を元に、WitnessStackの情報を更新する.
   * @param[in] request TransactionとWitnessStack追加情報を格納した構造体
   * @return Transactionのhexデータを格納した構造体
   */
  static UpdateWitnessStackResponseStruct UpdateWitnessStack(
      const UpdateWitnessStackRequestStruct& request);

  /**
   * @brief パラメータの情報を元に、SigHashを作成する
   * @param[in] request sighashを生成するパラメータ
   * @return sighashのhexデータを格納した構造体
   */
  static CreateElementsSignatureHashResponseStruct CreateSignatureHash(
      const CreateElementsSignatureHashRequestStruct& request);

  /**
   * @brief パラメータの情報を元に、RawTransactionをBlindする.
   * @param[in] request Blind対象のTransactionとBlind情報を格納した構造体
   * @return Transactionのhexデータを格納した構造体
   */
  static BlindRawTransactionResponseStruct BlindTransaction(
      const BlindRawTransactionRequestStruct& request);

  /**
   * @brief パラメータの情報を元に、RawTransactionをUnBlindする.
   * @param[in] request
   * Unlblind対象のTransactionとBlindingKey情報を格納した構造体
   * @return TransactionのhexデータとBlindingFactorを格納した構造体
   */
  static UnblindRawTransactionResponseStruct UnblindTransaction(
      const UnblindRawTransactionRequestStruct& request);

  /**
   * @brief パラメータの情報を元に、RawTransactionにAsset新規発行情報を設定する.
   * @param[in] request 設定対象のTransactionとAsset情報を格納した構造体
   * @return Transactionのhexデータを格納した構造体
   */
  static SetRawIssueAssetResponseStruct SetRawIssueAsset(
      const SetRawIssueAssetRequestStruct& request);

  /**
   * @brief パラメータの情報を元に、RawTransactionにAsset再発行情報を設定する.
   * @param[in] request 設定対象のTransactionとAsset情報を格納した構造体
   * @return Transactionのhexデータを格納した構造体
   */
  static SetRawReissueAssetResponseStruct SetRawReissueAsset(
      const SetRawReissueAssetRequestStruct& request);

  /**
   * @brief パラメータの情報を元に、Elements Pegin用のRaw Transactionを作成する.
   * @param[in] request Transactionを構築するパラメータの構造体
   * @return Transactionのhexデータを格納した構造体
   */
  static ElementsCreateRawPeginResponseStruct
  CreateRawPeginTransaction(  // NOLINT
      const ElementsCreateRawPeginRequestStruct& request);

  /**
   * @brief パラメータの情報を元に、Elements Pegout用のRaw Transactionを作成する.
   * @param[in] request Transactionを構築するパラメータの構造体
   * @return Transactionのhexデータを格納した構造体
   */
  static ElementsCreateRawPegoutResponseStruct CreateRawPegoutTransaction(
      const ElementsCreateRawPegoutRequestStruct& request);

  /**
   * @brief パラメータの情報を元に、Issue用BlindingKeyを作成する.
   * @param[in] request BlindingKeyを構築するパラメータの構造体
   * @return BlindingKeyを格納した構造体
   */
  static GetIssuanceBlindingKeyResponseStruct GetIssuanceBlindingKey(
      const GetIssuanceBlindingKeyRequestStruct& request);

  /**
   * @brief パラメータの情報を元に、Elements DestroyAmount用のRaw Transactionを作成する.
   * @param[in] request Transactionを構築するパラメータの構造体
   * @return Transactionのhexデータを格納した構造体
   */
  static ElementsCreateDestroyAmountResponseStruct
  CreateDestroyAmountTransaction(
      const ElementsCreateDestroyAmountRequestStruct& request);
};

}  // namespace api
}  // namespace js
}  // namespace cfd

#endif  // CFD_DISABLE_ELEMENTS
#endif  // CFD_INCLUDE_CFD_CFDAPI_ELEMENTS_TRANSACTION_H_
