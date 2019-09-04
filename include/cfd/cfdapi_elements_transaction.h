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

#include "cfd/cfd_common.h"
#include "cfd/cfdapi_struct.h"

/**
 * @brief cfdapi名前空間
 */
namespace cfd {
namespace api {

/**
 * @brief Elements用Transaction関連の関数群クラス
 */
class CFD_EXPORT ElementsTransactionApi {
 public:
  /**
   * @brief パラメータの情報を元に、Elements用のRaw Transactionを作成する.
   * @param[in] request Transactionを構築するパラメータの構造体
   * @return Transactionのhexデータを格納した構造体
   */
  static ElementsCreateRawTransactionResponseStruct CreateRawTransaction(
      const ElementsCreateRawTransactionRequestStruct& request);

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
   * @brief パラメータの情報を元に、Elements Pegin用のRaw Transactionを作成する.
   * @param[in] request Transactionを構築するパラメータの構造体
   * @return Transactionのhexデータを格納した構造体
   */
  static ElementsCreateRawPeginResponseStruct
  CreateRawPeginTransaction(  // NOLINT
      const ElementsCreateRawPeginRequestStruct& request);

 private:
  ElementsTransactionApi();
};

}  // namespace api
}  // namespace cfd

#endif  // CFD_DISABLE_ELEMENTS
#endif  // CFD_INCLUDE_CFD_CFDAPI_ELEMENTS_TRANSACTION_H_
