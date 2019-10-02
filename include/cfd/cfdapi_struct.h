// Copyright 2019 CryptoGarage
/**
 * @file cfdapi_struct.h
 *
 * @brief 構造体マッピングファイル (自動生成)
 */
#ifndef CFD_INCLUDE_CFD_CFDAPI_STRUCT_H_
#define CFD_INCLUDE_CFD_CFDAPI_STRUCT_H_

#include <cstdint>
#include <set>
#include <string>
#include <vector>

// clang-format off
// @formatter:off
namespace cfd {
namespace api {

// ------------------------------------------------------------------------
// InnerErrorResponseStruct
// ------------------------------------------------------------------------
/**
 * @brief InnerErrorResponseStruct 構造体
 */
struct InnerErrorResponseStruct {
  int64_t code = 0;          //!< code  // NOLINT
  std::string type = "";     //!< type  // NOLINT
  std::string message = "";  //!< message  // NOLINT
  std::set<std::string> ignore_items;   //!< using on JSON mapping convert.
};

// ------------------------------------------------------------------------
// MultisigSignDataStruct
// ------------------------------------------------------------------------
/**
 * @brief MultisigSignDataStruct 構造体
 */
struct MultisigSignDataStruct {
  std::string hex = "";                 //!< hex  // NOLINT
  bool der_encode = true;               //!< der_encode  // NOLINT
  std::string sighash_type = "all";     //!< sighash_type  // NOLINT
  bool sighash_anyone_can_pay = false;  //!< sighash_anyone_can_pay  // NOLINT
  std::string related_pubkey = "";      //!< related_pubkey  // NOLINT
  std::set<std::string> ignore_items;   //!< using on JSON mapping convert.
};

// ------------------------------------------------------------------------
// AddMultisigSignRequestStruct
// ------------------------------------------------------------------------
/**
 * @brief AddMultisigSignRequestStruct 構造体
 */
struct AddMultisigSignRequestStruct {
  bool is_elements = false;                         //!< is_elements  // NOLINT
  std::string tx_hex = "";                          //!< tx_hex  // NOLINT
  std::string txin_txid = "";                       //!< txin_txid  // NOLINT
  uint32_t txin_vout = 0;                           //!< txin_vout  // NOLINT
  std::vector<MultisigSignDataStruct> sign_params;  //!< sign_params  // NOLINT
  std::string redeem_script = "";                   //!< redeem_script  // NOLINT
  std::string witness_script = "";                  //!< witness_script  // NOLINT
  std::string txin_type = "";                       //!< txin_type  // NOLINT
  bool clear_stack = true;                          //!< clear_stack  // NOLINT
  std::set<std::string> ignore_items;   //!< using on JSON mapping convert.
};

// ------------------------------------------------------------------------
// AddMultisigSignResponseStruct
// ------------------------------------------------------------------------
/**
 * @brief AddMultisigSignResponseStruct 構造体
 */
struct AddMultisigSignResponseStruct {
  std::string hex = "";  //!< hex  // NOLINT
  InnerErrorResponseStruct error;       //!< error information
  std::set<std::string> ignore_items;   //!< using on JSON mapping convert.
};

// ------------------------------------------------------------------------
// SignDataStruct
// ------------------------------------------------------------------------
/**
 * @brief SignDataStruct 構造体
 */
struct SignDataStruct {
  std::string hex = "";                 //!< hex  // NOLINT
  std::string type = "binary";          //!< type  // NOLINT
  bool der_encode = false;              //!< der_encode  // NOLINT
  std::string sighash_type = "all";     //!< sighash_type  // NOLINT
  bool sighash_anyone_can_pay = false;  //!< sighash_anyone_can_pay  // NOLINT
  std::set<std::string> ignore_items;   //!< using on JSON mapping convert.
};

// ------------------------------------------------------------------------
// AddSignRequestStruct
// ------------------------------------------------------------------------
/**
 * @brief AddSignRequestStruct 構造体
 */
struct AddSignRequestStruct {
  std::string tx_hex = "";                 //!< tx_hex  // NOLINT
  bool is_elements = false;                //!< is_elements  // NOLINT
  std::string txin_txid = "";              //!< txin_txid  // NOLINT
  uint32_t txin_vout = 0;                  //!< txin_vout  // NOLINT
  bool is_witness = true;                  //!< is_witness  // NOLINT
  std::vector<SignDataStruct> sign_param;  //!< sign_param  // NOLINT
  bool clear_stack = true;                 //!< clear_stack  // NOLINT
  std::set<std::string> ignore_items;   //!< using on JSON mapping convert.
};

// ------------------------------------------------------------------------
// AddSignResponseStruct
// ------------------------------------------------------------------------
/**
 * @brief AddSignResponseStruct 構造体
 */
struct AddSignResponseStruct {
  std::string hex = "";  //!< hex  // NOLINT
  InnerErrorResponseStruct error;       //!< error information
  std::set<std::string> ignore_items;   //!< using on JSON mapping convert.
};

// ------------------------------------------------------------------------
// BlindTxInRequestStruct
// ------------------------------------------------------------------------
/**
 * @brief BlindTxInRequestStruct 構造体
 */
struct BlindTxInRequestStruct {
  std::string txid = "";                //!< txid  // NOLINT
  int64_t vout = 0;                     //!< vout  // NOLINT
  std::string asset = "";               //!< asset  // NOLINT
  std::string blind_factor = "";        //!< blind_factor  // NOLINT
  std::string asset_blind_factor = "";  //!< asset_blind_factor  // NOLINT
  int64_t amount = 0;                   //!< amount  // NOLINT
  std::set<std::string> ignore_items;   //!< using on JSON mapping convert.
};

// ------------------------------------------------------------------------
// BlindIssuanceRequestStruct
// ------------------------------------------------------------------------
/**
 * @brief BlindIssuanceRequestStruct 構造体
 */
struct BlindIssuanceRequestStruct {
  std::string txid = "";                //!< txid  // NOLINT
  int64_t vout = 0;                     //!< vout  // NOLINT
  std::string asset_blinding_key = "";  //!< asset_blinding_key  // NOLINT
  std::string token_blinding_key = "";  //!< token_blinding_key  // NOLINT
  std::set<std::string> ignore_items;   //!< using on JSON mapping convert.
};

// ------------------------------------------------------------------------
// BlindRawTransactionRequestStruct
// ------------------------------------------------------------------------
/**
 * @brief BlindRawTransactionRequestStruct 構造体
 */
struct BlindRawTransactionRequestStruct {
  std::string tx_hex = "";                            //!< tx_hex  // NOLINT
  std::vector<BlindTxInRequestStruct> txins;          //!< txins  // NOLINT
  std::vector<std::string> blind_pubkeys;             //!< blind_pubkeys  // NOLINT
  std::vector<BlindIssuanceRequestStruct> issuances;  //!< issuances  // NOLINT
  std::set<std::string> ignore_items;   //!< using on JSON mapping convert.
};

// ------------------------------------------------------------------------
// BlindRawTransactionResponseStruct
// ------------------------------------------------------------------------
/**
 * @brief BlindRawTransactionResponseStruct 構造体
 */
struct BlindRawTransactionResponseStruct {
  std::string hex = "";  //!< hex  // NOLINT
  InnerErrorResponseStruct error;       //!< error information
  std::set<std::string> ignore_items;   //!< using on JSON mapping convert.
};

// ------------------------------------------------------------------------
// CreateAddressRequestStruct
// ------------------------------------------------------------------------
/**
 * @brief CreateAddressRequestStruct 構造体
 */
struct CreateAddressRequestStruct {
  bool is_elements = false;          //!< is_elements  // NOLINT
  std::string script_hex = "";       //!< script_hex  // NOLINT
  std::string pubkey_hex = "";       //!< pubkey_hex  // NOLINT
  std::string network = "mainnet";   //!< network  // NOLINT
  std::string hash_type = "p2wpkh";  //!< hash_type  // NOLINT
  std::set<std::string> ignore_items;   //!< using on JSON mapping convert.
};

// ------------------------------------------------------------------------
// CreateAddressResponseStruct
// ------------------------------------------------------------------------
/**
 * @brief CreateAddressResponseStruct 構造体
 */
struct CreateAddressResponseStruct {
  std::string address = "";         //!< address  // NOLINT
  std::string locking_script = "";  //!< locking_script  // NOLINT
  std::string redeem_script = "";   //!< redeem_script  // NOLINT
  InnerErrorResponseStruct error;       //!< error information
  std::set<std::string> ignore_items;   //!< using on JSON mapping convert.
};

// ------------------------------------------------------------------------
// CreateKeyPairRequestStruct
// ------------------------------------------------------------------------
/**
 * @brief CreateKeyPairRequestStruct 構造体
 */
struct CreateKeyPairRequestStruct {
  bool wif = true;                  //!< wif  // NOLINT
  std::string network = "mainnet";  //!< network  // NOLINT
  bool is_compressed = true;        //!< is_compressed  // NOLINT
  std::set<std::string> ignore_items;   //!< using on JSON mapping convert.
};

// ------------------------------------------------------------------------
// CreateKeyPairResponseStruct
// ------------------------------------------------------------------------
/**
 * @brief CreateKeyPairResponseStruct 構造体
 */
struct CreateKeyPairResponseStruct {
  std::string privkey = "";  //!< privkey  // NOLINT
  std::string pubkey = "";   //!< pubkey  // NOLINT
  InnerErrorResponseStruct error;       //!< error information
  std::set<std::string> ignore_items;   //!< using on JSON mapping convert.
};

// ------------------------------------------------------------------------
// DecodeRawTransactionRequestStruct
// ------------------------------------------------------------------------
/**
 * @brief DecodeRawTransactionRequestStruct 構造体
 */
struct DecodeRawTransactionRequestStruct {
  std::string hex = "";             //!< hex  // NOLINT
  std::string network = "mainnet";  //!< network  // NOLINT
  bool iswitness = true;            //!< iswitness  // NOLINT
  std::set<std::string> ignore_items;   //!< using on JSON mapping convert.
};

// ------------------------------------------------------------------------
// DecodeUnlockingScriptStruct
// ------------------------------------------------------------------------
/**
 * @brief DecodeUnlockingScriptStruct 構造体
 */
struct DecodeUnlockingScriptStruct {
  std::string asm_ = "";  //!< asm_  // NOLINT
  std::string hex = "";   //!< hex  // NOLINT
  std::set<std::string> ignore_items;   //!< using on JSON mapping convert.
};

// ------------------------------------------------------------------------
// DecodeRawTransactionTxInStruct
// ------------------------------------------------------------------------
/**
 * @brief DecodeRawTransactionTxInStruct 構造体
 */
struct DecodeRawTransactionTxInStruct {
  std::string coinbase = "";               //!< coinbase  // NOLINT
  std::string txid = "";                   //!< txid  // NOLINT
  int64_t vout = 0;                        //!< vout  // NOLINT
  DecodeUnlockingScriptStruct script_sig;  //!< script_sig  // NOLINT
  std::vector<std::string> txinwitness;    //!< txinwitness  // NOLINT
  int64_t sequence = 0;                    //!< sequence  // NOLINT
  std::set<std::string> ignore_items;   //!< using on JSON mapping convert.
};

// ------------------------------------------------------------------------
// DecodeLockingScriptStruct
// ------------------------------------------------------------------------
/**
 * @brief DecodeLockingScriptStruct 構造体
 */
struct DecodeLockingScriptStruct {
  std::string asm_ = "";               //!< asm_  // NOLINT
  std::string hex = "";                //!< hex  // NOLINT
  int64_t req_sigs = 0;                //!< req_sigs  // NOLINT
  std::string type = "";               //!< type  // NOLINT
  std::vector<std::string> addresses;  //!< addresses  // NOLINT
  std::set<std::string> ignore_items;   //!< using on JSON mapping convert.
};

// ------------------------------------------------------------------------
// DecodeRawTransactionTxOutStruct
// ------------------------------------------------------------------------
/**
 * @brief DecodeRawTransactionTxOutStruct 構造体
 */
struct DecodeRawTransactionTxOutStruct {
  double value = 0;                          //!< value  // NOLINT
  int64_t n = 0;                             //!< n  // NOLINT
  DecodeLockingScriptStruct script_pub_key;  //!< script_pub_key  // NOLINT
  std::set<std::string> ignore_items;   //!< using on JSON mapping convert.
};

// ------------------------------------------------------------------------
// DecodeRawTransactionResponseStruct
// ------------------------------------------------------------------------
/**
 * @brief DecodeRawTransactionResponseStruct 構造体
 */
struct DecodeRawTransactionResponseStruct {
  std::string txid = "";                              //!< txid  // NOLINT
  std::string hash = "";                              //!< hash  // NOLINT
  uint32_t version = 0;                               //!< version  // NOLINT
  int64_t size = 0;                                   //!< size  // NOLINT
  int64_t vsize = 0;                                  //!< vsize  // NOLINT
  int64_t weight = 0;                                 //!< weight  // NOLINT
  uint32_t locktime = 0;                              //!< locktime  // NOLINT
  std::vector<DecodeRawTransactionTxInStruct> vin;    //!< vin  // NOLINT
  std::vector<DecodeRawTransactionTxOutStruct> vout;  //!< vout  // NOLINT
  InnerErrorResponseStruct error;       //!< error information
  std::set<std::string> ignore_items;   //!< using on JSON mapping convert.
};

// ------------------------------------------------------------------------
// CreateUnblindedAddressRequestStruct
// ------------------------------------------------------------------------
/**
 * @brief CreateUnblindedAddressRequestStruct 構造体
 */
struct CreateUnblindedAddressRequestStruct {
  std::string script_hex = "";                //!< script_hex  // NOLINT
  std::string pubkey_hex = "";                //!< pubkey_hex  // NOLINT
  std::string elements_network = "liquidv1";  //!< elements_network  // NOLINT
  std::set<std::string> ignore_items;   //!< using on JSON mapping convert.
};

// ------------------------------------------------------------------------
// CreateUnblindedAddressResponseStruct
// ------------------------------------------------------------------------
/**
 * @brief CreateUnblindedAddressResponseStruct 構造体
 */
struct CreateUnblindedAddressResponseStruct {
  std::string unblinded_address = "";  //!< unblinded_address  // NOLINT
  InnerErrorResponseStruct error;       //!< error information
  std::set<std::string> ignore_items;   //!< using on JSON mapping convert.
};

// ------------------------------------------------------------------------
// ElementsDestroyAmountTxInStruct
// ------------------------------------------------------------------------
/**
 * @brief ElementsDestroyAmountTxInStruct 構造体
 */
struct ElementsDestroyAmountTxInStruct {
  std::string txid = "";           //!< txid  // NOLINT
  uint32_t vout = 0;               //!< vout  // NOLINT
  uint32_t sequence = 4294967295;  //!< sequence  // NOLINT
  std::set<std::string> ignore_items;   //!< using on JSON mapping convert.
};

// ------------------------------------------------------------------------
// ElementsDestroyAmountTxOutStruct
// ------------------------------------------------------------------------
/**
 * @brief ElementsDestroyAmountTxOutStruct 構造体
 */
struct ElementsDestroyAmountTxOutStruct {
  std::string address = "";      //!< address  // NOLINT
  int64_t amount = 0;            //!< amount  // NOLINT
  std::string asset = "";        //!< asset  // NOLINT
  bool is_remove_nonce = false;  //!< is_remove_nonce  // NOLINT
  std::set<std::string> ignore_items;   //!< using on JSON mapping convert.
};

// ------------------------------------------------------------------------
// ElementsDestroyAmountStruct
// ------------------------------------------------------------------------
/**
 * @brief ElementsDestroyAmountStruct 構造体
 */
struct ElementsDestroyAmountStruct {
  int64_t amount = 0;      //!< amount  // NOLINT
  std::string asset = "";  //!< asset  // NOLINT
  std::set<std::string> ignore_items;   //!< using on JSON mapping convert.
};

// ------------------------------------------------------------------------
// ElementsDestroyAmountFeeStruct
// ------------------------------------------------------------------------
/**
 * @brief ElementsDestroyAmountFeeStruct 構造体
 */
struct ElementsDestroyAmountFeeStruct {
  int64_t amount = 0;      //!< amount  // NOLINT
  std::string asset = "";  //!< asset  // NOLINT
  std::set<std::string> ignore_items;   //!< using on JSON mapping convert.
};

// ------------------------------------------------------------------------
// ElementsCreateDestroyAmountRequestStruct
// ------------------------------------------------------------------------
/**
 * @brief ElementsCreateDestroyAmountRequestStruct 構造体
 */
struct ElementsCreateDestroyAmountRequestStruct {
  uint32_t version = 2;                                  //!< version  // NOLINT
  uint32_t locktime = 0;                                 //!< locktime  // NOLINT
  std::vector<ElementsDestroyAmountTxInStruct> txins;    //!< txins  // NOLINT
  std::vector<ElementsDestroyAmountTxOutStruct> txouts;  //!< txouts  // NOLINT
  ElementsDestroyAmountStruct destroy;                   //!< destroy  // NOLINT
  ElementsDestroyAmountFeeStruct fee;                    //!< fee  // NOLINT
  std::set<std::string> ignore_items;   //!< using on JSON mapping convert.
};

// ------------------------------------------------------------------------
// ElementsCreateDestroyAmountResponseStruct
// ------------------------------------------------------------------------
/**
 * @brief ElementsCreateDestroyAmountResponseStruct 構造体
 */
struct ElementsCreateDestroyAmountResponseStruct {
  std::string hex = "";  //!< hex  // NOLINT
  InnerErrorResponseStruct error;       //!< error information
  std::set<std::string> ignore_items;   //!< using on JSON mapping convert.
};

// ------------------------------------------------------------------------
// ElementsCreatePegInAddressRequestStruct
// ------------------------------------------------------------------------
/**
 * @brief ElementsCreatePegInAddressRequestStruct 構造体
 */
struct ElementsCreatePegInAddressRequestStruct {
  std::string fedpegscript = "";    //!< fedpegscript  // NOLINT
  std::string pubkey = "";          //!< pubkey  // NOLINT
  std::string network = "mainnet";  //!< network  // NOLINT
  std::set<std::string> ignore_items;   //!< using on JSON mapping convert.
};

// ------------------------------------------------------------------------
// ElementsCreatePegInAddressResponseStruct
// ------------------------------------------------------------------------
/**
 * @brief ElementsCreatePegInAddressResponseStruct 構造体
 */
struct ElementsCreatePegInAddressResponseStruct {
  std::string mainchain_address = "";   //!< mainchain_address  // NOLINT
  std::string claim_script = "";        //!< claim_script  // NOLINT
  std::string tweak_fedpegscript = "";  //!< tweak_fedpegscript  // NOLINT
  InnerErrorResponseStruct error;       //!< error information
  std::set<std::string> ignore_items;   //!< using on JSON mapping convert.
};

// ------------------------------------------------------------------------
// ElementsPeginWitnessStruct
// ------------------------------------------------------------------------
/**
 * @brief ElementsPeginWitnessStruct 構造体
 */
struct ElementsPeginWitnessStruct {
  int64_t amount = 0;                             //!< amount  // NOLINT
  std::string asset = "";                         //!< asset  // NOLINT
  std::string mainchain_genesis_block_hash = "";  //!< mainchain_genesis_block_hash  // NOLINT
  std::string claim_script = "";                  //!< claim_script  // NOLINT
  std::string mainchain_raw_transaction = "";     //!< mainchain_raw_transaction  // NOLINT
  std::string mainchain_txoutproof = "";          //!< mainchain_txoutproof  // NOLINT
  std::set<std::string> ignore_items;   //!< using on JSON mapping convert.
};

// ------------------------------------------------------------------------
// ElementsPeginTxInStruct
// ------------------------------------------------------------------------
/**
 * @brief ElementsPeginTxInStruct 構造体
 */
struct ElementsPeginTxInStruct {
  bool is_pegin = true;                         //!< is_pegin  // NOLINT
  std::string txid = "";                        //!< txid  // NOLINT
  uint32_t vout = 0;                            //!< vout  // NOLINT
  uint32_t sequence = 4294967295;               //!< sequence  // NOLINT
  ElementsPeginWitnessStruct peginwitness;      //!< peginwitness  // NOLINT
  bool is_remove_mainchain_tx_witness = false;  //!< is_remove_mainchain_tx_witness  // NOLINT
  std::set<std::string> ignore_items;   //!< using on JSON mapping convert.
};

// ------------------------------------------------------------------------
// ElementsPeginTxOutStruct
// ------------------------------------------------------------------------
/**
 * @brief ElementsPeginTxOutStruct 構造体
 */
struct ElementsPeginTxOutStruct {
  std::string address = "";      //!< address  // NOLINT
  int64_t amount = 0;            //!< amount  // NOLINT
  std::string asset = "";        //!< asset  // NOLINT
  bool is_remove_nonce = false;  //!< is_remove_nonce  // NOLINT
  std::set<std::string> ignore_items;   //!< using on JSON mapping convert.
};

// ------------------------------------------------------------------------
// ElementsPeginTxOutFeeStruct
// ------------------------------------------------------------------------
/**
 * @brief ElementsPeginTxOutFeeStruct 構造体
 */
struct ElementsPeginTxOutFeeStruct {
  int64_t amount = 0;      //!< amount  // NOLINT
  std::string asset = "";  //!< asset  // NOLINT
  std::set<std::string> ignore_items;   //!< using on JSON mapping convert.
};

// ------------------------------------------------------------------------
// ElementsCreateRawPeginRequestStruct
// ------------------------------------------------------------------------
/**
 * @brief ElementsCreateRawPeginRequestStruct 構造体
 */
struct ElementsCreateRawPeginRequestStruct {
  uint32_t version = 2;                          //!< version  // NOLINT
  uint32_t locktime = 0;                         //!< locktime  // NOLINT
  std::vector<ElementsPeginTxInStruct> txins;    //!< txins  // NOLINT
  std::vector<ElementsPeginTxOutStruct> txouts;  //!< txouts  // NOLINT
  ElementsPeginTxOutFeeStruct fee;               //!< fee  // NOLINT
  std::set<std::string> ignore_items;   //!< using on JSON mapping convert.
};

// ------------------------------------------------------------------------
// ElementsCreateRawPeginResponseStruct
// ------------------------------------------------------------------------
/**
 * @brief ElementsCreateRawPeginResponseStruct 構造体
 */
struct ElementsCreateRawPeginResponseStruct {
  std::string hex = "";  //!< hex  // NOLINT
  InnerErrorResponseStruct error;       //!< error information
  std::set<std::string> ignore_items;   //!< using on JSON mapping convert.
};

// ------------------------------------------------------------------------
// ElementsPegoutTxInStruct
// ------------------------------------------------------------------------
/**
 * @brief ElementsPegoutTxInStruct 構造体
 */
struct ElementsPegoutTxInStruct {
  std::string txid = "";           //!< txid  // NOLINT
  uint32_t vout = 0;               //!< vout  // NOLINT
  uint32_t sequence = 4294967295;  //!< sequence  // NOLINT
  std::set<std::string> ignore_items;   //!< using on JSON mapping convert.
};

// ------------------------------------------------------------------------
// ElementsPegoutTxOutStruct
// ------------------------------------------------------------------------
/**
 * @brief ElementsPegoutTxOutStruct 構造体
 */
struct ElementsPegoutTxOutStruct {
  std::string address = "";      //!< address  // NOLINT
  int64_t amount = 0;            //!< amount  // NOLINT
  std::string asset = "";        //!< asset  // NOLINT
  bool is_remove_nonce = false;  //!< is_remove_nonce  // NOLINT
  std::set<std::string> ignore_items;   //!< using on JSON mapping convert.
};

// ------------------------------------------------------------------------
// ElementsPegoutStruct
// ------------------------------------------------------------------------
/**
 * @brief ElementsPegoutStruct 構造体
 */
struct ElementsPegoutStruct {
  int64_t amount = 0;                             //!< amount  // NOLINT
  std::string asset = "";                         //!< asset  // NOLINT
  std::string network = "mainnet";                //!< network  // NOLINT
  std::string mainchain_genesis_block_hash = "";  //!< mainchain_genesis_block_hash  // NOLINT
  std::string btc_address = "";                   //!< btc_address  // NOLINT
  std::string online_pubkey = "";                 //!< online_pubkey  // NOLINT
  std::string master_online_key = "";             //!< master_online_key  // NOLINT
  std::string bitcoin_descriptor = "";            //!< bitcoin_descriptor  // NOLINT
  int64_t bip32_counter = 0;                      //!< bip32_counter  // NOLINT
  std::string whitelist = "";                     //!< whitelist  // NOLINT
  std::set<std::string> ignore_items;   //!< using on JSON mapping convert.
};

// ------------------------------------------------------------------------
// ElementsPegoutTxOutFeeStruct
// ------------------------------------------------------------------------
/**
 * @brief ElementsPegoutTxOutFeeStruct 構造体
 */
struct ElementsPegoutTxOutFeeStruct {
  int64_t amount = 0;      //!< amount  // NOLINT
  std::string asset = "";  //!< asset  // NOLINT
  std::set<std::string> ignore_items;   //!< using on JSON mapping convert.
};

// ------------------------------------------------------------------------
// ElementsCreateRawPegoutRequestStruct
// ------------------------------------------------------------------------
/**
 * @brief ElementsCreateRawPegoutRequestStruct 構造体
 */
struct ElementsCreateRawPegoutRequestStruct {
  uint32_t version = 2;                           //!< version  // NOLINT
  uint32_t locktime = 0;                          //!< locktime  // NOLINT
  std::vector<ElementsPegoutTxInStruct> txins;    //!< txins  // NOLINT
  std::vector<ElementsPegoutTxOutStruct> txouts;  //!< txouts  // NOLINT
  ElementsPegoutStruct pegout;                    //!< pegout  // NOLINT
  ElementsPegoutTxOutFeeStruct fee;               //!< fee  // NOLINT
  std::set<std::string> ignore_items;   //!< using on JSON mapping convert.
};

// ------------------------------------------------------------------------
// ElementsCreateRawPegoutResponseStruct
// ------------------------------------------------------------------------
/**
 * @brief ElementsCreateRawPegoutResponseStruct 構造体
 */
struct ElementsCreateRawPegoutResponseStruct {
  std::string hex = "";          //!< hex  // NOLINT
  std::string btc_address = "";  //!< btc_address  // NOLINT
  InnerErrorResponseStruct error;       //!< error information
  std::set<std::string> ignore_items;   //!< using on JSON mapping convert.
};

// ------------------------------------------------------------------------
// ElementsTxInRequestStruct
// ------------------------------------------------------------------------
/**
 * @brief ElementsTxInRequestStruct 構造体
 */
struct ElementsTxInRequestStruct {
  std::string txid = "";           //!< txid  // NOLINT
  uint32_t vout = 0;               //!< vout  // NOLINT
  uint32_t sequence = 4294967295;  //!< sequence  // NOLINT
  std::set<std::string> ignore_items;   //!< using on JSON mapping convert.
};

// ------------------------------------------------------------------------
// ElementsTxOutRequestStruct
// ------------------------------------------------------------------------
/**
 * @brief ElementsTxOutRequestStruct 構造体
 */
struct ElementsTxOutRequestStruct {
  std::string address = "";      //!< address  // NOLINT
  int64_t amount = 0;            //!< amount  // NOLINT
  std::string asset = "";        //!< asset  // NOLINT
  bool is_remove_nonce = false;  //!< is_remove_nonce  // NOLINT
  std::set<std::string> ignore_items;   //!< using on JSON mapping convert.
};

// ------------------------------------------------------------------------
// ElementsTxOutFeeRequestStruct
// ------------------------------------------------------------------------
/**
 * @brief ElementsTxOutFeeRequestStruct 構造体
 */
struct ElementsTxOutFeeRequestStruct {
  int64_t amount = 0;      //!< amount  // NOLINT
  std::string asset = "";  //!< asset  // NOLINT
  std::set<std::string> ignore_items;   //!< using on JSON mapping convert.
};

// ------------------------------------------------------------------------
// ElementsCreateRawTransactionRequestStruct
// ------------------------------------------------------------------------
/**
 * @brief ElementsCreateRawTransactionRequestStruct 構造体
 */
struct ElementsCreateRawTransactionRequestStruct {
  uint32_t version = 2;                            //!< version  // NOLINT
  uint32_t locktime = 0;                           //!< locktime  // NOLINT
  std::vector<ElementsTxInRequestStruct> txins;    //!< txins  // NOLINT
  std::vector<ElementsTxOutRequestStruct> txouts;  //!< txouts  // NOLINT
  ElementsTxOutFeeRequestStruct fee;               //!< fee  // NOLINT
  std::set<std::string> ignore_items;   //!< using on JSON mapping convert.
};

// ------------------------------------------------------------------------
// ElementsCreateRawTransactionResponseStruct
// ------------------------------------------------------------------------
/**
 * @brief ElementsCreateRawTransactionResponseStruct 構造体
 */
struct ElementsCreateRawTransactionResponseStruct {
  std::string hex = "";  //!< hex  // NOLINT
  InnerErrorResponseStruct error;       //!< error information
  std::set<std::string> ignore_items;   //!< using on JSON mapping convert.
};

// ------------------------------------------------------------------------
// ElementsDecodeRawTransactionRequestStruct
// ------------------------------------------------------------------------
/**
 * @brief ElementsDecodeRawTransactionRequestStruct 構造体
 */
struct ElementsDecodeRawTransactionRequestStruct {
  std::string hex = "";                //!< hex  // NOLINT
  std::string network = "liquidv1";    //!< network  // NOLINT
  std::string mainchain_network = "";  //!< mainchain_network  // NOLINT
  bool iswitness = true;               //!< iswitness  // NOLINT
  std::set<std::string> ignore_items;   //!< using on JSON mapping convert.
};

// ------------------------------------------------------------------------
// ElementsDecodeUnlockingScriptStruct
// ------------------------------------------------------------------------
/**
 * @brief ElementsDecodeUnlockingScriptStruct 構造体
 */
struct ElementsDecodeUnlockingScriptStruct {
  std::string asm_ = "";  //!< asm_  // NOLINT
  std::string hex = "";   //!< hex  // NOLINT
  std::set<std::string> ignore_items;   //!< using on JSON mapping convert.
};

// ------------------------------------------------------------------------
// ElementsDecodeIssuanceStruct
// ------------------------------------------------------------------------
/**
 * @brief ElementsDecodeIssuanceStruct 構造体
 */
struct ElementsDecodeIssuanceStruct {
  std::string asset_blinding_nonce = "";   //!< asset_blinding_nonce  // NOLINT
  std::string asset_entropy = "";          //!< asset_entropy  // NOLINT
  bool isreissuance = false;               //!< isreissuance  // NOLINT
  std::string token = "";                  //!< token  // NOLINT
  std::string asset = "";                  //!< asset  // NOLINT
  double assetamount = 0;                  //!< assetamount  // NOLINT
  std::string assetamountcommitment = "";  //!< assetamountcommitment  // NOLINT
  double tokenamount = 0;                  //!< tokenamount  // NOLINT
  std::string tokenamountcommitment = "";  //!< tokenamountcommitment  // NOLINT
  std::set<std::string> ignore_items;   //!< using on JSON mapping convert.
};

// ------------------------------------------------------------------------
// ElementsDecodeRawTransactionTxInStruct
// ------------------------------------------------------------------------
/**
 * @brief ElementsDecodeRawTransactionTxInStruct 構造体
 */
struct ElementsDecodeRawTransactionTxInStruct {
  std::string coinbase = "";                       //!< coinbase  // NOLINT
  std::string txid = "";                           //!< txid  // NOLINT
  int64_t vout = 0;                                //!< vout  // NOLINT
  ElementsDecodeUnlockingScriptStruct script_sig;  //!< script_sig  // NOLINT
  bool is_pegin = false;                           //!< is_pegin  // NOLINT
  int64_t sequence = 0;                            //!< sequence  // NOLINT
  std::vector<std::string> txinwitness;            //!< txinwitness  // NOLINT
  std::vector<std::string> pegin_witness;          //!< pegin_witness  // NOLINT
  ElementsDecodeIssuanceStruct issuance;           //!< issuance  // NOLINT
  std::set<std::string> ignore_items;   //!< using on JSON mapping convert.
};

// ------------------------------------------------------------------------
// ElementsDecodeLockingScriptStruct
// ------------------------------------------------------------------------
/**
 * @brief ElementsDecodeLockingScriptStruct 構造体
 */
struct ElementsDecodeLockingScriptStruct {
  std::string asm_ = "";                      //!< asm_  // NOLINT
  std::string hex = "";                       //!< hex  // NOLINT
  int64_t req_sigs = 0;                       //!< req_sigs  // NOLINT
  std::string type = "";                      //!< type  // NOLINT
  std::vector<std::string> addresses;         //!< addresses  // NOLINT
  std::string pegout_chain = "";              //!< pegout_chain  // NOLINT
  std::string pegout_asm = "";                //!< pegout_asm  // NOLINT
  std::string pegout_hex = "";                //!< pegout_hex  // NOLINT
  int64_t pegout_req_sigs = 0;                //!< pegout_req_sigs  // NOLINT
  std::string pegout_type = "";               //!< pegout_type  // NOLINT
  std::vector<std::string> pegout_addresses;  //!< pegout_addresses  // NOLINT
  std::set<std::string> ignore_items;   //!< using on JSON mapping convert.
};

// ------------------------------------------------------------------------
// ElementsDecodeRawTransactionTxOutStruct
// ------------------------------------------------------------------------
/**
 * @brief ElementsDecodeRawTransactionTxOutStruct 構造体
 */
struct ElementsDecodeRawTransactionTxOutStruct {
  double value = 0;                                  //!< value  // NOLINT
  double value_minimum = 0;                          //!< value_minimum  // NOLINT
  double value_maximum = 0;                          //!< value_maximum  // NOLINT
  int64_t ct_exponent = 0;                           //!< ct_exponent  // NOLINT
  int64_t ct_bits = 0;                               //!< ct_bits  // NOLINT
  std::string surjectionproof = "";                  //!< surjectionproof  // NOLINT
  std::string valuecommitment = "";                  //!< valuecommitment  // NOLINT
  std::string asset = "";                            //!< asset  // NOLINT
  std::string assetcommitment = "";                  //!< assetcommitment  // NOLINT
  std::string commitmentnonce = "";                  //!< commitmentnonce  // NOLINT
  bool commitmentnonce_fully_valid = false;          //!< commitmentnonce_fully_valid  // NOLINT
  int64_t n = 0;                                     //!< n  // NOLINT
  ElementsDecodeLockingScriptStruct script_pub_key;  //!< script_pub_key  // NOLINT
  std::set<std::string> ignore_items;   //!< using on JSON mapping convert.
};

// ------------------------------------------------------------------------
// ElementsDecodeRawTransactionResponseStruct
// ------------------------------------------------------------------------
/**
 * @brief ElementsDecodeRawTransactionResponseStruct 構造体
 */
struct ElementsDecodeRawTransactionResponseStruct {
  std::string txid = "";                                      //!< txid  // NOLINT
  std::string hash = "";                                      //!< hash  // NOLINT
  std::string wtxid = "";                                     //!< wtxid  // NOLINT
  std::string withash = "";                                   //!< withash  // NOLINT
  uint32_t version = 0;                                       //!< version  // NOLINT
  int64_t size = 0;                                           //!< size  // NOLINT
  int64_t vsize = 0;                                          //!< vsize  // NOLINT
  int64_t weight = 0;                                         //!< weight  // NOLINT
  uint32_t locktime = 0;                                      //!< locktime  // NOLINT
  std::vector<ElementsDecodeRawTransactionTxInStruct> vin;    //!< vin  // NOLINT
  std::vector<ElementsDecodeRawTransactionTxOutStruct> vout;  //!< vout  // NOLINT
  InnerErrorResponseStruct error;       //!< error information
  std::set<std::string> ignore_items;   //!< using on JSON mapping convert.
};

// ------------------------------------------------------------------------
// GetConfidentialAddressRequestStruct
// ------------------------------------------------------------------------
/**
 * @brief GetConfidentialAddressRequestStruct 構造体
 */
struct GetConfidentialAddressRequestStruct {
  std::string unblinded_address = "";  //!< unblinded_address  // NOLINT
  std::string key = "";                //!< key  // NOLINT
  std::set<std::string> ignore_items;   //!< using on JSON mapping convert.
};

// ------------------------------------------------------------------------
// GetConfidentialAddressResponseStruct
// ------------------------------------------------------------------------
/**
 * @brief GetConfidentialAddressResponseStruct 構造体
 */
struct GetConfidentialAddressResponseStruct {
  std::string confidential_address = "";  //!< confidential_address  // NOLINT
  InnerErrorResponseStruct error;       //!< error information
  std::set<std::string> ignore_items;   //!< using on JSON mapping convert.
};

// ------------------------------------------------------------------------
// GetUnblindedAddressRequestStruct
// ------------------------------------------------------------------------
/**
 * @brief GetUnblindedAddressRequestStruct 構造体
 */
struct GetUnblindedAddressRequestStruct {
  std::string confidential_address = "";  //!< confidential_address  // NOLINT
  std::set<std::string> ignore_items;   //!< using on JSON mapping convert.
};

// ------------------------------------------------------------------------
// GetUnblindedAddressResponseStruct
// ------------------------------------------------------------------------
/**
 * @brief GetUnblindedAddressResponseStruct 構造体
 */
struct GetUnblindedAddressResponseStruct {
  std::string unblinded_address = "";  //!< unblinded_address  // NOLINT
  InnerErrorResponseStruct error;       //!< error information
  std::set<std::string> ignore_items;   //!< using on JSON mapping convert.
};

// ------------------------------------------------------------------------
// IssuanceDataRequestStruct
// ------------------------------------------------------------------------
/**
 * @brief IssuanceDataRequestStruct 構造体
 */
struct IssuanceDataRequestStruct {
  std::string txin_txid = "";      //!< txin_txid  // NOLINT
  uint32_t txin_vout = 0;          //!< txin_vout  // NOLINT
  int64_t asset_amount = 0;        //!< asset_amount  // NOLINT
  std::string asset_address = "";  //!< asset_address  // NOLINT
  int64_t token_amount = 0;        //!< token_amount  // NOLINT
  std::string token_address = "";  //!< token_address  // NOLINT
  bool is_blind = true;            //!< is_blind  // NOLINT
  std::string contract_hash = "";  //!< contract_hash  // NOLINT
  bool is_remove_nonce = false;    //!< is_remove_nonce  // NOLINT
  std::set<std::string> ignore_items;   //!< using on JSON mapping convert.
};

// ------------------------------------------------------------------------
// SetRawIssueAssetRequestStruct
// ------------------------------------------------------------------------
/**
 * @brief SetRawIssueAssetRequestStruct 構造体
 */
struct SetRawIssueAssetRequestStruct {
  std::string tx_hex = "";                           //!< tx_hex  // NOLINT
  bool is_randomize = false;                         //!< is_randomize  // NOLINT
  std::vector<IssuanceDataRequestStruct> issuances;  //!< issuances  // NOLINT
  std::set<std::string> ignore_items;   //!< using on JSON mapping convert.
};

// ------------------------------------------------------------------------
// IssuanceDataResponseStruct
// ------------------------------------------------------------------------
/**
 * @brief IssuanceDataResponseStruct 構造体
 */
struct IssuanceDataResponseStruct {
  std::string txin_txid = "";  //!< txin_txid  // NOLINT
  uint32_t txin_vout = 0;      //!< txin_vout  // NOLINT
  std::string asset = "";      //!< asset  // NOLINT
  std::string entropy = "";    //!< entropy  // NOLINT
  std::string token = "";      //!< token  // NOLINT
  InnerErrorResponseStruct error;       //!< error information
  std::set<std::string> ignore_items;   //!< using on JSON mapping convert.
};

// ------------------------------------------------------------------------
// SetRawIssueAssetResponseStruct
// ------------------------------------------------------------------------
/**
 * @brief SetRawIssueAssetResponseStruct 構造体
 */
struct SetRawIssueAssetResponseStruct {
  std::string hex = "";                               //!< hex  // NOLINT
  std::vector<IssuanceDataResponseStruct> issuances;  //!< issuances  // NOLINT
  InnerErrorResponseStruct error;       //!< error information
  std::set<std::string> ignore_items;   //!< using on JSON mapping convert.
};

// ------------------------------------------------------------------------
// ReissuanceDataRequestStruct
// ------------------------------------------------------------------------
/**
 * @brief ReissuanceDataRequestStruct 構造体
 */
struct ReissuanceDataRequestStruct {
  std::string txin_txid = "";             //!< txin_txid  // NOLINT
  uint32_t txin_vout = 0;                 //!< txin_vout  // NOLINT
  int64_t amount = 0;                     //!< amount  // NOLINT
  std::string address = "";               //!< address  // NOLINT
  std::string asset_blinding_nonce = "";  //!< asset_blinding_nonce  // NOLINT
  std::string asset_entropy = "";         //!< asset_entropy  // NOLINT
  bool is_remove_nonce = false;           //!< is_remove_nonce  // NOLINT
  std::set<std::string> ignore_items;   //!< using on JSON mapping convert.
};

// ------------------------------------------------------------------------
// SetRawReissueAssetRequestStruct
// ------------------------------------------------------------------------
/**
 * @brief SetRawReissueAssetRequestStruct 構造体
 */
struct SetRawReissueAssetRequestStruct {
  std::string tx_hex = "";                             //!< tx_hex  // NOLINT
  bool is_randomize = false;                           //!< is_randomize  // NOLINT
  std::vector<ReissuanceDataRequestStruct> issuances;  //!< issuances  // NOLINT
  std::set<std::string> ignore_items;   //!< using on JSON mapping convert.
};

// ------------------------------------------------------------------------
// ReissuanceDataResponseStruct
// ------------------------------------------------------------------------
/**
 * @brief ReissuanceDataResponseStruct 構造体
 */
struct ReissuanceDataResponseStruct {
  std::string txin_txid = "";  //!< txin_txid  // NOLINT
  uint32_t txin_vout = 0;      //!< txin_vout  // NOLINT
  std::string asset = "";      //!< asset  // NOLINT
  std::string entropy = "";    //!< entropy  // NOLINT
  InnerErrorResponseStruct error;       //!< error information
  std::set<std::string> ignore_items;   //!< using on JSON mapping convert.
};

// ------------------------------------------------------------------------
// SetRawReissueAssetResponseStruct
// ------------------------------------------------------------------------
/**
 * @brief SetRawReissueAssetResponseStruct 構造体
 */
struct SetRawReissueAssetResponseStruct {
  std::string hex = "";                                 //!< hex  // NOLINT
  std::vector<ReissuanceDataResponseStruct> issuances;  //!< issuances  // NOLINT
  InnerErrorResponseStruct error;       //!< error information
  std::set<std::string> ignore_items;   //!< using on JSON mapping convert.
};

// ------------------------------------------------------------------------
// UnblindIssuanceStruct
// ------------------------------------------------------------------------
/**
 * @brief UnblindIssuanceStruct 構造体
 */
struct UnblindIssuanceStruct {
  std::string txid = "";                //!< txid  // NOLINT
  int64_t vout = 0;                     //!< vout  // NOLINT
  std::string asset_blinding_key = "";  //!< asset_blinding_key  // NOLINT
  std::string token_blinding_key = "";  //!< token_blinding_key  // NOLINT
  std::set<std::string> ignore_items;   //!< using on JSON mapping convert.
};

// ------------------------------------------------------------------------
// UnblindRawTransactionRequestStruct
// ------------------------------------------------------------------------
/**
 * @brief UnblindRawTransactionRequestStruct 構造体
 */
struct UnblindRawTransactionRequestStruct {
  std::string tx_hex = "";                       //!< tx_hex  // NOLINT
  int64_t target_output_index = -1;              //!< target_output_index  // NOLINT
  std::vector<std::string> blinding_keys;        //!< blinding_keys  // NOLINT
  std::vector<UnblindIssuanceStruct> issuances;  //!< issuances  // NOLINT
  std::set<std::string> ignore_items;   //!< using on JSON mapping convert.
};

// ------------------------------------------------------------------------
// UnblindOutputStruct
// ------------------------------------------------------------------------
/**
 * @brief UnblindOutputStruct 構造体
 */
struct UnblindOutputStruct {
  std::string asset = "";               //!< asset  // NOLINT
  std::string blind_factor = "";        //!< blind_factor  // NOLINT
  std::string asset_blind_factor = "";  //!< asset_blind_factor  // NOLINT
  int64_t amount = 0;                   //!< amount  // NOLINT
  std::set<std::string> ignore_items;   //!< using on JSON mapping convert.
};

// ------------------------------------------------------------------------
// UnblindIssuanceOutputStruct
// ------------------------------------------------------------------------
/**
 * @brief UnblindIssuanceOutputStruct 構造体
 */
struct UnblindIssuanceOutputStruct {
  std::string txid = "";    //!< txid  // NOLINT
  int64_t vout = 0;         //!< vout  // NOLINT
  std::string asset = "";   //!< asset  // NOLINT
  int64_t assetamount = 0;  //!< assetamount  // NOLINT
  std::string token = "";   //!< token  // NOLINT
  int64_t tokenamount = 0;  //!< tokenamount  // NOLINT
  std::set<std::string> ignore_items;   //!< using on JSON mapping convert.
};

// ------------------------------------------------------------------------
// UnblindRawTransactionResponseStruct
// ------------------------------------------------------------------------
/**
 * @brief UnblindRawTransactionResponseStruct 構造体
 */
struct UnblindRawTransactionResponseStruct {
  std::string hex = "";                                       //!< hex  // NOLINT
  std::vector<UnblindOutputStruct> outputs;                   //!< outputs  // NOLINT
  std::vector<UnblindIssuanceOutputStruct> issuance_outputs;  //!< issuance_outputs  // NOLINT
  InnerErrorResponseStruct error;       //!< error information
  std::set<std::string> ignore_items;   //!< using on JSON mapping convert.
};

// ------------------------------------------------------------------------
// GetIssuanceBlindingKeyRequestStruct
// ------------------------------------------------------------------------
/**
 * @brief GetIssuanceBlindingKeyRequestStruct 構造体
 */
struct GetIssuanceBlindingKeyRequestStruct {
  std::string master_blinding_key = "";  //!< master_blinding_key  // NOLINT
  std::string txid = "";                 //!< txid  // NOLINT
  uint32_t vout = 0;                     //!< vout  // NOLINT
  std::set<std::string> ignore_items;   //!< using on JSON mapping convert.
};

// ------------------------------------------------------------------------
// GetIssuanceBlindingKeyResponseStruct
// ------------------------------------------------------------------------
/**
 * @brief GetIssuanceBlindingKeyResponseStruct 構造体
 */
struct GetIssuanceBlindingKeyResponseStruct {
  std::string blinding_key = "";  //!< blinding_key  // NOLINT
  InnerErrorResponseStruct error;       //!< error information
  std::set<std::string> ignore_items;   //!< using on JSON mapping convert.
};

// ------------------------------------------------------------------------
// GetMnemonicWordlistRequestStruct
// ------------------------------------------------------------------------
/**
 * @brief GetMnemonicWordlistRequestStruct 構造体
 */
struct GetMnemonicWordlistRequestStruct {
  std::string language = "en";  //!< language  // NOLINT
  std::set<std::string> ignore_items;   //!< using on JSON mapping convert.
};

// ------------------------------------------------------------------------
// GetMnemonicWordlistResponseStruct
// ------------------------------------------------------------------------
/**
 * @brief GetMnemonicWordlistResponseStruct 構造体
 */
struct GetMnemonicWordlistResponseStruct {
  std::vector<std::string> wordlist;  //!< wordlist  // NOLINT
  InnerErrorResponseStruct error;       //!< error information
  std::set<std::string> ignore_items;   //!< using on JSON mapping convert.
};

// ------------------------------------------------------------------------
// GetWitnessStackNumRequestStruct
// ------------------------------------------------------------------------
/**
 * @brief GetWitnessStackNumRequestStruct 構造体
 */
struct GetWitnessStackNumRequestStruct {
  std::string tx_hex = "";     //!< tx_hex  // NOLINT
  bool is_elements = false;    //!< is_elements  // NOLINT
  std::string txin_txid = "";  //!< txin_txid  // NOLINT
  uint32_t txin_vout = 0;      //!< txin_vout  // NOLINT
  std::set<std::string> ignore_items;   //!< using on JSON mapping convert.
};

// ------------------------------------------------------------------------
// GetWitnessStackNumResponseStruct
// ------------------------------------------------------------------------
/**
 * @brief GetWitnessStackNumResponseStruct 構造体
 */
struct GetWitnessStackNumResponseStruct {
  int64_t count = 0;  //!< count  // NOLINT
  InnerErrorResponseStruct error;       //!< error information
  std::set<std::string> ignore_items;   //!< using on JSON mapping convert.
};

// ------------------------------------------------------------------------
// CreateMultisigRequestStruct
// ------------------------------------------------------------------------
/**
 * @brief CreateMultisigRequestStruct 構造体
 */
struct CreateMultisigRequestStruct {
  uint8_t nrequired = 1;               //!< nrequired  // NOLINT
  std::vector<std::string> keys;       //!< keys  // NOLINT
  bool is_elements = false;            //!< is_elements  // NOLINT
  std::string network = "mainnet";     //!< network  // NOLINT
  std::string address_type = "p2wsh";  //!< address_type  // NOLINT
  std::set<std::string> ignore_items;   //!< using on JSON mapping convert.
};

// ------------------------------------------------------------------------
// CreateMultisigResponseStruct
// ------------------------------------------------------------------------
/**
 * @brief CreateMultisigResponseStruct 構造体
 */
struct CreateMultisigResponseStruct {
  std::string address = "";         //!< address  // NOLINT
  std::string redeem_script = "";   //!< redeem_script  // NOLINT
  std::string witness_script = "";  //!< witness_script  // NOLINT
  InnerErrorResponseStruct error;       //!< error information
  std::set<std::string> ignore_items;   //!< using on JSON mapping convert.
};

// ------------------------------------------------------------------------
// CreateSignatureHashRequestStruct
// ------------------------------------------------------------------------
/**
 * @brief CreateSignatureHashRequestStruct 構造体
 */
struct CreateSignatureHashRequestStruct {
  std::string tx_hex = "";              //!< tx_hex  // NOLINT
  std::string txin_txid = "";           //!< txin_txid  // NOLINT
  uint32_t txin_vout = 0;               //!< txin_vout  // NOLINT
  std::string script_hex = "";          //!< script_hex  // NOLINT
  std::string pubkey_hex = "";          //!< pubkey_hex  // NOLINT
  int64_t amount = 0;                   //!< amount  // NOLINT
  std::string hash_type = "p2wsh";      //!< hash_type  // NOLINT
  std::string sighash_type = "all";     //!< sighash_type  // NOLINT
  bool sighash_anyone_can_pay = false;  //!< sighash_anyone_can_pay  // NOLINT
  std::set<std::string> ignore_items;   //!< using on JSON mapping convert.
};

// ------------------------------------------------------------------------
// CreateSignatureHashResponseStruct
// ------------------------------------------------------------------------
/**
 * @brief CreateSignatureHashResponseStruct 構造体
 */
struct CreateSignatureHashResponseStruct {
  std::string sighash = "";  //!< sighash  // NOLINT
  InnerErrorResponseStruct error;       //!< error information
  std::set<std::string> ignore_items;   //!< using on JSON mapping convert.
};

// ------------------------------------------------------------------------
// CreateElementsSignatureHashRequestStruct
// ------------------------------------------------------------------------
/**
 * @brief CreateElementsSignatureHashRequestStruct 構造体
 */
struct CreateElementsSignatureHashRequestStruct {
  std::string tx_hex = "";                  //!< tx_hex  // NOLINT
  std::string txin_txid = "";               //!< txin_txid  // NOLINT
  uint32_t txin_vout = 0;                   //!< txin_vout  // NOLINT
  std::string script_hex = "";              //!< script_hex  // NOLINT
  std::string pubkey_hex = "";              //!< pubkey_hex  // NOLINT
  int64_t amount = 0;                       //!< amount  // NOLINT
  std::string confidential_value_hex = "";  //!< confidential_value_hex  // NOLINT
  std::string hash_type = "p2wsh";          //!< hash_type  // NOLINT
  std::string sighash_type = "all";         //!< sighash_type  // NOLINT
  bool sighash_anyone_can_pay = false;      //!< sighash_anyone_can_pay  // NOLINT
  std::set<std::string> ignore_items;   //!< using on JSON mapping convert.
};

// ------------------------------------------------------------------------
// CreateElementsSignatureHashResponseStruct
// ------------------------------------------------------------------------
/**
 * @brief CreateElementsSignatureHashResponseStruct 構造体
 */
struct CreateElementsSignatureHashResponseStruct {
  std::string sighash = "";  //!< sighash  // NOLINT
  InnerErrorResponseStruct error;       //!< error information
  std::set<std::string> ignore_items;   //!< using on JSON mapping convert.
};

// ------------------------------------------------------------------------
// GetSupportedFunctionResponseStruct
// ------------------------------------------------------------------------
/**
 * @brief GetSupportedFunctionResponseStruct 構造体
 */
struct GetSupportedFunctionResponseStruct {
  bool bitcoin = false;   //!< bitcoin  // NOLINT
  bool elements = false;  //!< elements  // NOLINT
  InnerErrorResponseStruct error;       //!< error information
  std::set<std::string> ignore_items;   //!< using on JSON mapping convert.
};

// ------------------------------------------------------------------------
// TxInRequestStruct
// ------------------------------------------------------------------------
/**
 * @brief TxInRequestStruct 構造体
 */
struct TxInRequestStruct {
  std::string txid = "";              //!< txid  // NOLINT
  uint32_t vout = 0;                  //!< vout  // NOLINT
  std::string data = "";              //!< data  // NOLINT
  std::string addr_type = "mainnet";  //!< addr_type  // NOLINT
  uint32_t sequence = 4294967295;     //!< sequence  // NOLINT
  std::set<std::string> ignore_items;   //!< using on JSON mapping convert.
};

// ------------------------------------------------------------------------
// TxOutRequestStruct
// ------------------------------------------------------------------------
/**
 * @brief TxOutRequestStruct 構造体
 */
struct TxOutRequestStruct {
  std::string address = "";  //!< address  // NOLINT
  int64_t amount = 0;        //!< amount  // NOLINT
  std::set<std::string> ignore_items;   //!< using on JSON mapping convert.
};

// ------------------------------------------------------------------------
// CreateRawTransactionRequestStruct
// ------------------------------------------------------------------------
/**
 * @brief CreateRawTransactionRequestStruct 構造体
 */
struct CreateRawTransactionRequestStruct {
  uint32_t version = 2;                    //!< version  // NOLINT
  uint32_t locktime = 0;                   //!< locktime  // NOLINT
  std::vector<TxInRequestStruct> txins;    //!< txins  // NOLINT
  std::vector<TxOutRequestStruct> txouts;  //!< txouts  // NOLINT
  std::set<std::string> ignore_items;   //!< using on JSON mapping convert.
};

// ------------------------------------------------------------------------
// CreateRawTransactionResponseStruct
// ------------------------------------------------------------------------
/**
 * @brief CreateRawTransactionResponseStruct 構造体
 */
struct CreateRawTransactionResponseStruct {
  std::string hex = "";  //!< hex  // NOLINT
  InnerErrorResponseStruct error;       //!< error information
  std::set<std::string> ignore_items;   //!< using on JSON mapping convert.
};

// ------------------------------------------------------------------------
// WitnessStackDataStruct
// ------------------------------------------------------------------------
/**
 * @brief WitnessStackDataStruct 構造体
 */
struct WitnessStackDataStruct {
  uint64_t index = 0;                   //!< index  // NOLINT
  std::string hex = "";                 //!< hex  // NOLINT
  std::string type = "binary";          //!< type  // NOLINT
  bool der_encode = false;              //!< der_encode  // NOLINT
  std::string sighash_type = "all";     //!< sighash_type  // NOLINT
  bool sighash_anyone_can_pay = false;  //!< sighash_anyone_can_pay  // NOLINT
  std::set<std::string> ignore_items;   //!< using on JSON mapping convert.
};

// ------------------------------------------------------------------------
// UpdateWitnessStackRequestStruct
// ------------------------------------------------------------------------
/**
 * @brief UpdateWitnessStackRequestStruct 構造体
 */
struct UpdateWitnessStackRequestStruct {
  std::string tx_hex = "";               //!< tx_hex  // NOLINT
  bool is_elements = false;              //!< is_elements  // NOLINT
  std::string txin_txid = "";            //!< txin_txid  // NOLINT
  uint32_t txin_vout = 0;                //!< txin_vout  // NOLINT
  WitnessStackDataStruct witness_stack;  //!< witness_stack  // NOLINT
  std::set<std::string> ignore_items;   //!< using on JSON mapping convert.
};

// ------------------------------------------------------------------------
// UpdateWitnessStackResponseStruct
// ------------------------------------------------------------------------
/**
 * @brief UpdateWitnessStackResponseStruct 構造体
 */
struct UpdateWitnessStackResponseStruct {
  std::string hex = "";  //!< hex  // NOLINT
  InnerErrorResponseStruct error;       //!< error information
  std::set<std::string> ignore_items;   //!< using on JSON mapping convert.
};

}  // namespace api
}  // namespace cfd

namespace dlc {
namespace api {

// ------------------------------------------------------------------------
// CETxSignDataStruct
// ------------------------------------------------------------------------
/**
 * @brief CETxSignDataStruct 構造体
 */
struct CETxSignDataStruct {
  std::string hex = "";                 //!< hex  // NOLINT
  bool der_encode = true;               //!< der_encode  // NOLINT
  std::string sighash_type = "all";     //!< sighash_type  // NOLINT
  bool sighash_anyone_can_pay = false;  //!< sighash_anyone_can_pay  // NOLINT
  std::set<std::string> ignore_items;   //!< using on JSON mapping convert.
};

// ------------------------------------------------------------------------
// AddCETxSignRequestStruct
// ------------------------------------------------------------------------
/**
 * @brief AddCETxSignRequestStruct 構造体
 */
struct AddCETxSignRequestStruct {
  std::string tx_hex = "";         //!< tx_hex  // NOLINT
  std::string txin_txid = "";      //!< txin_txid  // NOLINT
  uint32_t txin_vout = 0;          //!< txin_vout  // NOLINT
  CETxSignDataStruct sign;         //!< sign  // NOLINT
  bool delayed_unlocking = false;  //!< delayed_unlocking  // NOLINT
  std::string redeem_script = "";  //!< redeem_script  // NOLINT
  std::set<std::string> ignore_items;   //!< using on JSON mapping convert.
};

// ------------------------------------------------------------------------
// AddCETxSignResponseStruct
// ------------------------------------------------------------------------
/**
 * @brief AddCETxSignResponseStruct 構造体
 */
struct AddCETxSignResponseStruct {
  std::string hex = "";  //!< hex  // NOLINT
  cfd::api::InnerErrorResponseStruct error;   //!< error information
  std::set<std::string> ignore_items;   //!< using on JSON mapping convert.
};

// ------------------------------------------------------------------------
// CombineKeysRequestStruct
// ------------------------------------------------------------------------
/**
 * @brief CombineKeysRequestStruct 構造体
 */
struct CombineKeysRequestStruct {
  std::string pubkey = "";          //!< pubkey  // NOLINT
  std::string commitment_key = "";  //!< commitment_key  // NOLINT
  std::set<std::string> ignore_items;   //!< using on JSON mapping convert.
};

// ------------------------------------------------------------------------
// CreateCETxAddressRequestStruct
// ------------------------------------------------------------------------
/**
 * @brief CreateCETxAddressRequestStruct 構造体
 */
struct CreateCETxAddressRequestStruct {
  std::string network = "mainnet";        //!< network  // NOLINT
  CombineKeysRequestStruct combine_keys;  //!< combine_keys  // NOLINT
  std::string counter_party_pubkey = "";  //!< counter_party_pubkey  // NOLINT
  int64_t delay = 0;                      //!< delay  // NOLINT
  std::set<std::string> ignore_items;   //!< using on JSON mapping convert.
};

// ------------------------------------------------------------------------
// CreateCETxAddressResponseStruct
// ------------------------------------------------------------------------
/**
 * @brief CreateCETxAddressResponseStruct 構造体
 */
struct CreateCETxAddressResponseStruct {
  std::string address = "";          //!< address  // NOLINT
  std::string redeem_script = "";    //!< redeem_script  // NOLINT
  std::string combined_pubkey = "";  //!< combined_pubkey  // NOLINT
  cfd::api::InnerErrorResponseStruct error;   //!< error information
  std::set<std::string> ignore_items;   //!< using on JSON mapping convert.
};

}  // namespace api
}  // namespace dlc

// @formatter:on
// clang-format on

#endif  // CFD_INCLUDE_CFD_CFDAPI_STRUCT_H_
