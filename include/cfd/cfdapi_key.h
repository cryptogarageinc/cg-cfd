// Copyright 2019 CryptoGarage
/**
 * @file cfdapi_key.h
 *
 * @brief cfd-apiで利用する鍵関連のクラス定義
 *
 * 構造体形式のAPIを提供する.
 */
#ifndef CFD_INCLUDE_CFD_CFDAPI_KEY_H_
#define CFD_INCLUDE_CFD_CFDAPI_KEY_H_

#include <string>

#include "cfd/cfd_common.h"
#include "cfd/cfdapi_struct.h"

namespace cfd {
namespace api {

using cfd::core::NetType;
using cfd::core::Privkey;
using cfd::core::Pubkey;

/**
 * @brief 鍵情報関連のAPI群クラス
 */
class CFD_EXPORT KeyApi {
 public:
  /**
   * @brief constructor
   */
  KeyApi() {}

  /**
   * @brief 秘密鍵と公開鍵のペアを生成する.
   * @param[in] is_compressed 圧縮pubkeyかどうか
   * @param[out] pubkey pubkeyオブジェクト
   * @param[out] wif WIF文字列
   * @param[in] net_type mainnet/testnet/regtest/liquidv1
   * @return Privkeyオブジェクト
   */
  Privkey CreateKeyPair(
      bool is_compressed, Pubkey* pubkey, std::string* wif = nullptr,
      NetType net_type = NetType::kMainnet);
};
}  // namespace api
}  // namespace cfd

/**
 * @brief cfdapi名前空間
 */
namespace cfd {
namespace js {
namespace api {

/**
 * @brief 鍵情報関連の関数群クラス
 */
class CFD_EXPORT KeyStructApi {
 public:
  /**
   * @brief 秘密鍵と公開鍵のペアを生成する.
   * @param[in] request key pairを構築するパラメータ
   * @return privkeyとpubkeyのデータを格納した構造体
   */
  static CreateKeyPairResponseStruct CreateKeyPair(
      const CreateKeyPairRequestStruct& request);

  /**
   * @brief ec signatureを生成する.
   * @param[in] request ec signatureを計算するためのパラメータ
   * @return ec signature
   */
  static CalculateEcSignatureResponseStruct CalculateEcSignature(
      const CalculateEcSignatureRequestStruct& request);

 private:
  KeyStructApi();
};

}  // namespace api
}  // namespace js
}  // namespace cfd

#endif  // CFD_INCLUDE_CFD_CFDAPI_KEY_H_
