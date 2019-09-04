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
#include "cfd/cfdapi_struct.h"
#include "cfdcore/cfdcore_bytedata.h"
#include "cfdcore/cfdcore_script.h"
#include "cfdcore/cfdcore_util.h"

/**
 * @brief cfdapi名前空間
 */
namespace cfd {
namespace api {

/**
 * @brief Transaction関連のJSON APIクラス
 */
class CFD_EXPORT TransactionApi {
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
   * @brief SigHashTypeを文字列から変換する。
   * @param[in] sighash_type_string   SigHashTypeの文字列
   * @param[in] is_anyone_can_pay     anyone_can_payかどうか
   * @return SigHashType値
   */
  static cfdcore::SigHashType ConvertSigHashType(
      const std::string& sighash_type_string, bool is_anyone_can_pay);

  // internal API
  /**
   * @brief Multisigのredeem scriptからsignに必要なpublic keyを取得する.
   * @details redeem scriptにOP_CHECKMULTISIG(VERIFY)が複数含まれる場合は、
   *   末尾に近いOP_CHECKMULTISIGに必要なpublic keyのみを返却する.
   * @param[in] multisig_script Multisigのredeem script
   * @return signatureの順序を保ったpublic keyの配列
   */
  static const std::vector<cfdcore::Pubkey>
  ExtractPubkeysFromMultisigScript(  // NOLINT
      const cfdcore::Script& multisig_script);

  /**
   * @brief 署名情報の変換処理を行います
   * @param[in] hex_string              署名情報
   * @param[in] is_sign                 署名データかどうか
   * @param[in] is_der_encode           DERエンコード指定かどうか
   * @param[in] sighash_type            SigHash種別
   * @param[in] sighash_anyone_can_pay  SigHashのAnyoneCanPayフラグ
   * @return 変換後の署名情報
   */
  static cfdcore::ByteData ConvertSignDataToSignature(
      const std::string& hex_string, bool is_sign, bool is_der_encode,
      const std::string& sighash_type, bool sighash_anyone_can_pay);

 private:
  TransactionApi();

  /**
   * @brief MultiSigスクリプトかどうかをチェックする。
   * @param[in] script    スクリプト
   * @retval true   MultiSig
   * @retval false  その他のスクリプト
   */
  static bool CheckMultiSigScript(const cfdcore::Script& script);
  /**
   * @brief P2PKHスクリプトかどうかをチェックする。
   * @param[in] script    スクリプト
   * @retval true   P2PKH
   * @retval false  その他のスクリプト
   */
  static bool CheckP2pkhScript(const cfdcore::Script& script);
  /**
   * @brief P2SHスクリプトかどうかをチェックする。
   * @param[in] script    スクリプト
   * @retval true   P2SH
   * @retval false  その他のスクリプト
   */
  static bool CheckP2shScript(const cfdcore::Script& script);
  /**
   * @brief Pubkeyスクリプトかどうかをチェックする。
   * @param[in] script    スクリプト
   * @retval true   Pubkeyスクリプト
   * @retval false  その他のスクリプト
   */
  static bool CheckPubkeyScript(const cfdcore::Script& script);
  /**
   * @brief NullDataスクリプトかどうかをチェックする。
   * @param[in] script    スクリプト
   * @retval true   NullDataスクリプト
   * @retval false  その他のスクリプト
   */
  static bool CheckNullDataScript(const cfdcore::Script& script);
};

}  // namespace api
}  // namespace cfd

#endif  // CFD_INCLUDE_CFD_CFDAPI_TRANSACTION_H_
