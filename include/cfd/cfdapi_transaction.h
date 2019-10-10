// Copyright 2019 CryptoGarage
/**
 * @file cfdapi_transaction.h
 *
 * @brief cfd-apiで利用するTransaction作成のクラス定義
 *
 * JSON形式のAPIを提供する.
 */
#ifndef CFD_INCLUDE_CFD_CFDAPI_TRANSACTION_H_
#define CFD_INCLUDE_CFD_CFDAPI_TRANSACTION_H_

#include <string>
#include <vector>

#include "cfd/cfd_common.h"
#include "cfd/cfd_transaction.h"
#include "cfd/cfdapi_struct.h"
#include "cfdcore/cfdcore_bytedata.h"
#include "cfdcore/cfdcore_script.h"
#include "cfdcore/cfdcore_util.h"

namespace cfd {
namespace api {

using cfd::TransactionController;
using cfd::core::Amount;
using cfd::core::ByteData;
using cfd::core::HashType;
using cfd::core::Pubkey;
using cfd::core::Script;
using cfd::core::SigHashType;
using cfd::core::TxIn;
using cfd::core::TxInReference;
using cfd::core::TxOut;

/**
 * @brief Transaction関連のAPIクラス
 */
class CFD_EXPORT TransactionApi {
 public:
  /**
   * @brief constructor
   */
  TransactionApi(){};

  TransactionController CreateRawTransaction(
      uint32_t version, uint32_t locktime, const std::vector<TxIn>& txins,
      const std::vector<TxOut>& txouts) const;

  ByteData CreateSignatureHash(
      const std::string& tx_hex, const TxInReference& txin,
      const Pubkey& pubkey, const Amount& amount, HashType hash_type,
      const SigHashType& sighash_type) const;

  ByteData CreateSignatureHash(
      const std::string& tx_hex, const TxInReference& txin,
      const Script& redeem_script, const Amount& amount, HashType hash_type,
      const SigHashType& sighash_type) const;

  ByteData CreateSignatureHash(
      const std::string& tx_hex, const TxInReference& txin,
      const ByteData& key_data, const Amount& amount, HashType hash_type,
      const SigHashType& sighash_type) const;
};
}  // namespace api
}  // namespace cfd

namespace cfd {
namespace js {
namespace api {

/**
 * @brief Transaction関連のJSON APIクラス
 */
class CFD_EXPORT TransactionStructApi {
 public:
  /**
   * @brief JSONパラメータの情報を元に、Transactionを作成する.
   * @param[in] request Transactionを構築するパラメータの構造体
   * @return Transactionのhexデータを格納した構造体
   */
  static CreateRawTransactionResponseStruct CreateRawTransaction(
      const CreateRawTransactionRequestStruct& request);

  /**
   * @brief JSONパラメータの情報を元に、Transactionをデコードして出力する.
   * @param[in] request Transactionとデコード用の情報を格納した構造体
   * @return Transactionの表示用JSONデータを格納した構造体
   */
  static DecodeRawTransactionResponseStruct DecodeRawTransaction(
      const DecodeRawTransactionRequestStruct& request);

  /**
   * @brief JSONパラメータの情報を元に、WitnessStack数を出力する.
   * @param[in] request Transactionと対象TxIn情報を格納した構造体
   * @return WitnessStack数を格納した構造体
   */
  static GetWitnessStackNumResponseStruct GetWitnessStackNum(
      const GetWitnessStackNumRequestStruct& request);

  /**
   * @brief JSONパラメータの情報を元に、署名情報を追加する.
   * @param[in] request Transactionと署名情報を格納した構造体
   * @return Transactionのhexデータを格納した構造体
   */
  static AddSignResponseStruct AddSign(const AddSignRequestStruct& request);

  /**
   * @brief JSONパラメータの情報を元に、WitnessStackの情報を更新する.
   * @param[in] request TransactionとWitnessStack追加情報を格納した構造体
   * @return Transactionのhexデータを格納した構造体
   */
  static UpdateWitnessStackResponseStruct UpdateWitnessStack(
      const UpdateWitnessStackRequestStruct& request);

  /**
   * @brief JSONパラメータの情報を元に、SegwitのMultisig署名情報を追加する.
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
   * @brief JSONパラメータの情報を元に、SigHashを作成する
   * @param[in] request sighashを生成するパラメータ
   * @return sighashのhexデータを格納した構造体
   */
  static CreateSignatureHashResponseStruct CreateSignatureHash(
      const CreateSignatureHashRequestStruct& request);

 private:
  TransactionStructApi();

  /**
   * @brief MultiSigスクリプトかどうかをチェックする。
   * @param[in] script    スクリプト
   * @retval true   MultiSig
   * @retval false  その他のスクリプト
   */
  static bool CheckMultiSigScript(const cfd::core::Script& script);
  /**
   * @brief P2PKHスクリプトかどうかをチェックする。
   * @param[in] script    スクリプト
   * @retval true   P2PKH
   * @retval false  その他のスクリプト
   */
  static bool CheckP2pkhScript(const cfd::core::Script& script);
  /**
   * @brief P2SHスクリプトかどうかをチェックする。
   * @param[in] script    スクリプト
   * @retval true   P2SH
   * @retval false  その他のスクリプト
   */
  static bool CheckP2shScript(const cfd::core::Script& script);
  /**
   * @brief Pubkeyスクリプトかどうかをチェックする。
   * @param[in] script    スクリプト
   * @retval true   Pubkeyスクリプト
   * @retval false  その他のスクリプト
   */
  static bool CheckPubkeyScript(const cfd::core::Script& script);
  /**
   * @brief NullDataスクリプトかどうかをチェックする。
   * @param[in] script    スクリプト
   * @retval true   NullDataスクリプト
   * @retval false  その他のスクリプト
   */
  static bool CheckNullDataScript(const cfd::core::Script& script);
};

}  // namespace api
}  // namespace js
}  // namespace cfd

#endif  // CFD_INCLUDE_CFD_CFDAPI_TRANSACTION_H_
