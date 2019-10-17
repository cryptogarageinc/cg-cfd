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
using cfd::core::BlindParameter;
using cfd::core::ByteData;
using cfd::core::ConfidentialAssetId;
using cfd::core::ConfidentialTxIn;
using cfd::core::ConfidentialTxInReference;
using cfd::core::ConfidentialTxOut;
using cfd::core::ConfidentialValue;
using cfd::core::HashType;
using cfd::core::IssuanceBlindingKeyPair;
using cfd::core::IssuanceParameter;
using cfd::core::Privkey;
using cfd::core::Pubkey;
using cfd::core::Script;
using cfd::core::SigHashType;
using cfd::core::Txid;

/**
 * @brief TxIn Blinding parameters
 */
struct TxInBlindParameters {
  Txid txid;                             //!< txid
  uint32_t vout;                         //!< vout
  BlindParameter blind_param;            //!< blinding parameter
  bool is_issuance;                      //!< issuance flag
  IssuanceBlindingKeyPair issuance_key;  //!< issuance blinding keys
};

/**
 * @brief TxOut Blinding keys
 */
struct TxOutBlindKeys {
  uint32_t index;       //!< txout index
  Pubkey blinding_key;  //!< blinding key
};

/**
 * @brief TxIn pegin parameters
 */
struct TxInPeginParameters {
  Txid txid;                      //!< txid
  uint32_t vout;                  //!< vout
  Amount amount;                  //!< amount
  ConfidentialAssetId asset;      //!< asset
  BlockHash mainchain_blockhash;  //!< mainchain genesis block hash
  Script claim_script;            //!< claim script
  /**
   * @brief mainchain raw transaction.
   * @see ConfidentialTransaction::GetBitcoinTransaction
   */
  ByteData mainchain_raw_tx;
  ByteData mainchain_txoutproof;  //!< mainchain txoutproof
};

/**
 * @brief TxOut pegout parameters
 */
struct TxOutPegoutParameters {
  Amount amount;                   //!< amount
  ConfidentialAssetId asset;       //!< asset
  BlockHash genesisblock_hash;     //!< mainchain genesis block hash
  Address btc_address;             //!< mainchain bitcoin address
  NetType net_type;                //!< mainchain network type
  Pubkey online_pubkey;            //!< online pubkey
  Privkey master_online_key;       //!< master online key
  std::string bitcoin_descriptor;  //!< bitcoin descriptor
  uint32_t bip32_counter;          //!< bip32 counter
  ByteData whitelist;              //!< claim script
};

/**
 * @brief TxOut Unblinding keys
 */
struct TxOutUnblindKeys {
  uint32_t index;        //!< txout index
  Privkey blinding_key;  //!< blinding key
};

/**
 * @brief Issuance Blinding keys
 */
struct IssuanceBlindKeys {
  Txid txid;                             //!< txid
  uint32_t vout;                         //!< vout
  IssuanceBlindingKeyPair issuance_key;  //!< issuance blinding keys
};

/**
 * @brief Unblind output
 */
struct UnblindOutputs {
  uint32_t index;              //!< txout index
  BlindParameter blind_param;  //!< blinding parameter
};

/**
 * @brief Issuance Unblind output
 */
struct UnblindIssuanceOutputs {
  Txid txid;                       //!< txid
  uint32_t vout;                   //!< vout
  ConfidentialAssetId asset;       //!< asset id
  ConfidentialValue asset_amount;  //!< asset amount
  ConfidentialAssetId token;       //!< token asset id
  ConfidentialValue token_amount;  //!< token amount
};

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
   * @brief hexで与えられたtxに、SignDataを付与した
   *     ConfidentialTransctionControllerを作成する.
   * @param[in] tx_hex          tx hex string
   * @param[in] txid            target tx input txid
   * @param[in] vout            target tx input vout
   * @param[in] sign_params     sign data list
   * @param[in] is_witness      use witness
   * @param[in] clear_stack     clear stack data before add.
   * @return SignDataが付与されたTransactionController
   */
  ConfidentialTransactionController AddSign(
      const std::string& tx_hex, const Txid& txid, const uint32_t vout,
      const std::vector<SignParameter>& sign_params, bool is_witness = true,
      bool clear_stack = false) const;

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
   * @param[in] sign_list       sign data list
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

  /**
   * @brief Elements用RawTransactionをBlindする.
   * @param[in] tx_hex                 transaction hex string
   * @param[in] txin_blind_keys        txin blinding data
   * @param[in] txout_blind_keys       txout blinding data
   * @param[in] is_issuance_blinding   issuance有無
   * @return Transaction
   */
  ConfidentialTransactionController BlindTransaction(
      const std::string& tx_hex,
      const std::vector<TxInBlindParameters>& txin_blind_keys,
      const std::vector<TxOutBlindKeys>& txout_blind_keys,
      bool is_issuance_blinding = false);

  /**
   * @brief Elements用RawTransactionをUnblindする.
   * @param[in]  tx_hex                 transaction hex string
   * @param[in]  txout_unblind_keys     txout blinding data
   * @param[in]  issuance_blind_keys    issuance blinding data
   * @param[out] blind_outputs          blind parameter
   * @param[out] issuance_outputs       issuance parameter
   * @return Transaction
   */
  ConfidentialTransactionController UnblindTransaction(
      const std::string& tx_hex,
      const std::vector<TxOutUnblindKeys>& txout_unblind_keys,
      const std::vector<IssuanceBlindKeys>& issuance_blind_keys,
      std::vector<UnblindOutputs>* blind_outputs,
      std::vector<UnblindIssuanceOutputs>* issuance_outputs);

  /**
   *
   * @return
   */
  ConfidentialTransactionController SetRawIssueAsset();

  /**
   *
   * @return
   */
  ConfidentialTransactionController SetRawReissueAsset();

  /**
   * @brief Elements用のRaw Pegin Transactionを作成する.
   * @param[in] version     tx version
   * @param[in] locktime    lock time
   * @param[in] txins       tx input list
   * @param[in] pegins      tx pegin input list
   * @param[in] txouts      tx output list
   * @param[in] txout_fee   tx output fee
   * @return Pegin Transaction
   */
  ConfidentialTransactionController CreateRawPeginTransaction(
      uint32_t version, uint32_t locktime,
      const std::vector<ConfidentialTxIn>& txins,
      const std::vector<TxInPeginParameters>& pegins,
      const std::vector<ConfidentialTxOut>& txouts,
      const ConfidentialTxOut& txout_fee) const;

  /**
   * @brief Elements用のRaw Pegout Transactionを作成する.
   * @param[in] version     tx version
   * @param[in] locktime    lock time
   * @param[in] txins       tx input list
   * @param[in] txouts      tx output list
   * @param[in] pegout_data tx pegout data
   * @param[in] txout_fee   tx output fee
   * @param[out] pegout_address   pegout address
   * @return Pegout Transaction
   */
  ConfidentialTransactionController CreateRawPegoutTransaction(
      uint32_t version, uint32_t locktime,
      const std::vector<ConfidentialTxIn>& txins,
      const std::vector<ConfidentialTxOut>& txouts,
      const TxOutPegoutParameters& pegout_data,
      const ConfidentialTxOut& txout_fee,
      Address* pegout_address = nullptr) const;

  /**
   *
   * @return
   */
  uint32_t GetWitnessStackNum();

  /**
   *
   * @return
   */
  ConfidentialTransactionController UpdateWitnessStack();

  /**
   * @brief Issue用BlindingKeyを作成する.
   * @param[in] master_blinding_key master blindingKey
   * @param[in] txid                issuance utxo txid
   * @param[in] vout                issuance utxo vout
   * @return blinding key
   */
  Privkey GetIssuanceBlindingKey(
      const Privkey& master_blinding_key, const Txid& txid, int32_t vout);

  // CreateDestroyAmountTransaction
  // see CreateRawTransaction and ConfidentialTxOut::CreateDestroyAmountTxOut
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
