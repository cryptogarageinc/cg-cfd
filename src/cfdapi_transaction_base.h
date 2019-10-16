// Copyright 2019 CryptoGarage
/**
 * @file cfdapi_transaction_base.h
 *
 * @brief cfd-apiで利用するTransaction作成のクラス定義
 *
 * JSON形式のAPIを提供する.
 */
#ifndef CFD_SRC_CFDAPI_TRANSACTION_BASE_H_
#define CFD_SRC_CFDAPI_TRANSACTION_BASE_H_

#include <functional>
#include <string>
#include <vector>

#include "cfd/cfd_common.h"
#include "cfd/cfd_elements_transaction.h"
#include "cfd/cfd_transaction.h"
#include "cfd/cfdapi_struct.h"
#include "cfdcore/cfdcore_address.h"
#include "cfdcore/cfdcore_bytedata.h"
#include "cfdcore/cfdcore_key.h"
#include "cfdcore/cfdcore_transaction_common.h"
#include "cfdcore/cfdcore_util.h"

/**
 * @brief cfdapi namespace
 */
namespace cfd {
namespace api {

using cfd::core::AbstractTxInReference;
using cfd::core::AddressType;
using cfd::core::ByteData;
using cfd::core::Pubkey;
using cfd::core::Script;
using cfd::core::Txid;

/**
 * @brief Class providing common functionalities to TransactionStructApi and
 * ConfidentialTransactionApi.
 */
class TransactionApiBase {
 public:
  /**
   * @brief Add signature information based on parameter information.
   * @param[in] create_controller   transaction controller create function
   * @param[in] hex                 transaction hex
   * @param[in] txid                txid of input to add sign parameters to
   * @param[in] vout                vout of input to add sign parameters to
   * @param[in] sign_params         sign parameters to add the input
   * @param[in] is_witness          flag to add sign parameters to
   *     witness or unlocking script
   * @param[in] clear_stack         flag of clear all stacks
   * @return structure containing Transaction hex data
   */
  template <class T>
  static T AddSign(
      std::function<T(const std::string&)> create_controller,
      const std::string& hex, const Txid& txid, const uint32_t vout,
      const std::vector<SignParameter>& sign_params, bool is_witness = true,
      bool clear_stack = true);

 public:
  /**
   * @brief Add Segwit multisig signature information.
   * @details the order of signatures to be added is automatically aligned
   * with the corresponding pubkey in redeemscript and relatedPubkey in
   * signParam. (If relatedPubkey is not set, signatures are added in order of
   * signParam after adding signature with relatedPubkey).
   * @param[in] tx_hex          tx hex string
   * @param[in] txin            target tx input
   * @param[in] sign_list       value (amount or commitment)
   * @param[in] address_type    address type. (support is P2sh-P2wsh or P2wsh)
   * @param[in] witness_script  witness script
   * @param[in] redeem_script   redeem script
   * @param[in] clear_stack     clear stack data before add.
   * @param[in] create_controller a callback to create a transaction controller.
   * @return structure that holds Transaction and Segwit multisig signature
   * information.
   */
  template <class T>
  static std::string AddMultisigSign(
      const std::string& tx_hex, const AbstractTxInReference& txin,
      const std::vector<SignParameter>& sign_list, AddressType address_type,
      const Script& witness_script, const Script redeem_script,
      bool clear_stack,
      std::function<T(const std::string&)> create_controller);
};

}  // namespace api
}  // namespace cfd

namespace cfd {
namespace js {
namespace api {

using cfd::core::ByteData;
using cfd::core::Pubkey;
using cfd::core::Script;

/**
 * @typedef LockingScriptType
 * @brief LockingScript の Script Type 定義
 */
enum LockingScriptType {
  kNonStandard = 0,      //!< non standard locking script
  kPayToPubkey,          //!< p2pk locking script
  kPayToPubkeyHash,      //!< p2pkh locking script
  kPayToScriptHash,      //!< p2sh locking script
  kMultisig,             //!< multisig locking script
  kNullData,             //!< null data of locking script
  kWitnessV0ScriptHash,  //!< p2wsh locking script
  kWitnessV0KeyHash,     //!< p2wpkh locking script
  kWitnessUnknown,       //!< invalid witness ver locking script
  kTrue,                 //!< can spend anyone script
  kFee,                  //!< type fee
};

/**
 * @brief LockingScriptの解析情報
 */
struct ExtractScriptData {
  LockingScriptType script_type;  //!< LockingScript種別
  std::vector<ByteData> pushed_datas;  //!< LockingScriptに含まれるhashデータ
  int64_t req_sigs;                    //!< Unlockingに必要なSignature数
};

/**
 * @brief Class providing common functionalities to TransactionStructApi and
 * ConfidentialTransactionApi.
 */
class TransactionStructApiBase {
 public:
  /**
   * @brief Outputs number of elements in witness stack based on JSON parameters.
   * @param[in] request structure containing Transaction and TxIn information.
   * @param[in] create_controller a callback to create a transaction controller.
   * @return number of elements in the witness stack.
   */
  template <class T>
  static GetWitnessStackNumResponseStruct GetWitnessStackNum(
      const GetWitnessStackNumRequestStruct& request,
      std::function<T(const std::string&)> create_controller);

  /**
   * @brief Update witness stack based on JSON parameters.
   * @param[in] request structure containing Transaction and witness stack
   * information.
   * @param[in] create_controller a callback to create a transaction controller.
   * @return Transactionのhexデータを格納した構造体
   */
  template <class T>
  static UpdateWitnessStackResponseStruct UpdateWitnessStack(
      const UpdateWitnessStackRequestStruct& request,
      std::function<T(const std::string&)> create_controller);

  /**
   * @brief Convert signature information to a signature.
   * @param[in] hex_string              Signature information
   * @param[in] is_sign                 Whether signature data is provided
   * @param[in] is_der_encode           Whether the signature is DER encoded
   * @param[in] sighash_type            SigHash type
   * @param[in] sighash_anyone_can_pay  Flag determining if SigHash is
   * anyone_can_pay
   * @return Converted signature information.
   */
  static ByteData ConvertSignDataToSignature(
      const std::string& hex_string, bool is_sign, bool is_der_encode,
      const std::string& sighash_type, bool sighash_anyone_can_pay);

  /**
   * @brief Convert a string to a SigHashType object.
   * @param[in] sighash_type_string   SigHashType as a string
   * @param[in] is_anyone_can_pay     whether or not anyone_can_pay is used
   * @return SigHashType object
   */
  static cfd::core::SigHashType ConvertSigHashType(
      const std::string& sighash_type_string, bool is_anyone_can_pay);

  /**
   * @brief LockingScriptの解析を行う.
   * @param[in] locking_script LockingScriptデータ
   * @return 解析された ExtractScriptData インスタンス
   */
  static ExtractScriptData ExtractLockingScript(Script locking_script);

  /**
   * @brief LockingScriptType を文字列情報に変換する.
   * @param[in] script_type LockingScriptType インスタンス
   * @return LockingScriptType に応じたscript_type文字列
   */
  static std::string ConvertLockingScriptTypeString(
      LockingScriptType script_type);

  /**
   * @brief Convert SignDataStruct to SignParameter object.
   * @param[in] sign_data   SignDataStruct which is passed by StructApi request.
   * @return a SignParameter instance
   */
  template <class SignStructClass>
  static SignParameter ConvertSignDataStructToSignParameter(
      const SignStructClass& sign_data);
};

}  // namespace api
}  // namespace js
}  // namespace cfd

#endif  // CFD_SRC_CFDAPI_TRANSACTION_BASE_H_