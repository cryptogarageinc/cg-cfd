// Copyright 2019 CryptoGarage
/**
 * @file cfd_script.h
 *
 * @brief Script生成Utilクラス定義
 */
#ifndef CFD_INCLUDE_CFD_CFD_SCRIPT_H_
#define CFD_INCLUDE_CFD_CFD_SCRIPT_H_

#include <vector>

#include "cfd/cfd_common.h"
#include "cfdcore/cfdcore_bytedata.h"
#include "cfdcore/cfdcore_coin.h"
#include "cfdcore/cfdcore_key.h"
#include "cfdcore/cfdcore_script.h"

/**
 * @brief cfd 名前空間
 */
namespace cfd {

using cfdcore::BlockHash;
using cfdcore::ByteData;
using cfdcore::ByteData160;
using cfdcore::ByteData256;
using cfdcore::Pubkey;
using cfdcore::Script;

/**
 * @brief Scriptを作成する関数群クラス
 */
class CFD_EXPORT ScriptUtil {
 public:
  /**
   * @brief P2PKのlocking scriptを作成する.
   * @param[in] pubkey Pubkeyインスタンス
   * @return Scriptインスタンス
   * @details 下記の内容のScriptを作成する.
   * @code{.unparse}
   * <pubkey> OP_CHECKSIG
   * @endcode
   */
  static Script CreateP2pkLockingScript(const Pubkey& pubkey);
  /**
   * @brief P2PKHのlocking scriptを作成する.
   * @param[in] pubkey_hash pubkey hashが格納されたByteData160インスタンス
   * @return Scriptインスタンス
   * @details 下記の内容のScriptを作成する.
   * @code{.unparse}
   * OP_DUP OP_HASH160 <hash160(pubkey)> OP_EQUALVERIFY OP_CHECKSIG
   * @endcode
   */
  static Script CreateP2pkhLockingScript(const ByteData160& pubkey_hash);
  /**
   * @brief P2PKHのlocking scriptを作成する.
   * @param[in] pubkey Pubkeyインスタンス
   * @return Scriptインスタンス
   * @details 下記の内容のScriptを作成する.
   * @code{.unparse}
   * OP_DUP OP_HASH160 <hash160(pubkey)> OP_EQUALVERIFY OP_CHECKSIG
   * @endcode
   */
  static Script CreateP2pkhLockingScript(const Pubkey& pubkey);
  /**
   * @brief P2SHのlocking scriptを作成する.
   * @param[in] script_hash script hashが格納されたByteData160インスタンス
   * @return Scriptインスタンス
   * @details 下記の内容のScriptを作成する.
   * @code{.unparse}
   * OP_HASH160 <hash160(redeemScript)> OP_EQUAL
   * @endcode
   */
  static Script CreateP2shLockingScript(const ByteData160& script_hash);
  /**
   * @brief P2SHのlocking scriptを作成する.
   * @param[in] redeem_script redeem scriptのScriptインスタンス
   * @return Scriptインスタンス
   * @details 下記の内容のScriptを作成する.
   * @code{.unparse}
   * OP_HASH160 <hash160(redeemScript)> OP_EQUAL
   * @endcode
   */
  static Script CreateP2shLockingScript(const Script& redeem_script);
  /**
   * @brief P2WPKHのlocking scriptを作成する.
   * @param[in] pubkey_hash pubkey hashが格納されたByteData160インスタンス
   * @return Scriptインスタンス
   * @details 下記の内容のScriptを作成する.
   * @code{.unparse}
   * OP_0 <hash160(pubkey)>
   * @endcode
   */
  static Script CreateP2wpkhLockingScript(const ByteData160& pubkey_hash);
  /**
   * @brief P2WPKHのlocking scriptを作成する.
   * @param[in] pubkey Pubkeyインスタンス
   * @return Scriptインスタンス
   * @details 下記の内容のScriptを作成する.
   * @code{.unparse}
   * OP_0 <hash160(pubkey)>
   * @endcode
   */
  static Script CreateP2wpkhLockingScript(const Pubkey& pubkey);
  /**
   * @brief P2WSHのlocking scriptを作成する.
   * @param[in] script_hash script hashのByteData256インスタンス
   * @return Scriptインスタンス
   * @details 下記の内容のScriptを作成する.
   * @code{.unparse}
   * OP_0 <sha256(redeemScript)>
   * @endcode
   */
  static Script CreateP2wshLockingScript(const ByteData256& script_hash);
  /**
   * @brief P2WSHのlocking scriptを作成する.
   * @param[in] redeem_script redeem scriptのScriptインスタンス
   * @return Scriptインスタンス
   * @details 下記の内容のScriptを作成する.
   * @code{.unparse}
   * OP_0 <sha256(redeemScript)>
   * @endcode
   */
  static Script CreateP2wshLockingScript(const Script& redeem_script);
  /**
   * @brief RedeemScriptが有効なものであるかをチェックする.
   * @param[in] redeem_script redeem script
   * @retval true 有効なredeem script
   * @retval false 有効でないredeem script
   */
  static bool IsValidRedeemScript(const Script& redeem_script);
  /**
   * @brief M-of-N Multisigのredeem scriptを作成する.
   * @param[in] require_sig_num unlockingに必要なSignature数（Mに相当）
   * @param[in] pubkeys 署名に対応するPubkey配列（Nに相当）
   * @return Scriptインスタンス
   * @details 下記の内容のScriptを作成する.
   * @code{.unparse}
   * OP_n <pubkey> ... OP_<requireSigNum> OP_CHECKMULTISIG
   * @endcode
   */
  static Script CreateMultisigRedeemScript(
      uint32_t require_sig_num, const std::vector<Pubkey>& pubkeys);

  /**
   * @brief Pegoutのlocking scriptを作成する.
   * @param[in] genesisblock_hash mainchainのgenesisblock hash
   * @param[in] script_pubkey 送り先のscript pubkey
   * @param[in] btc_pubkey_bytes DerivePubTweak関数で作られたpubkey情報
   * @param[in] whitelist_proof whitelistの証明
   * @return Scriptインスタンス
   */
  static Script CreatePegoutLogkingScript(
      const BlockHash& genesisblock_hash, const Script& script_pubkey,
      const Pubkey& btc_pubkey_bytes, const ByteData& whitelist_proof);

 private:
  ScriptUtil();
};

}  // namespace cfd

#endif  // CFD_INCLUDE_CFD_CFD_SCRIPT_H_
