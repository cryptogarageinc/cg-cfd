#include "gtest/gtest.h"
#include <vector>

#include "cfd/cfd_common.h"
#include "cfd/cfd_utxo.h"
#include "cfdcore/cfdcore_address.h"
#include "cfdcore/cfdcore_bytedata.h"
#include "cfdcore/cfdcore_coin.h"
#include "cfdcore/cfdcore_elements_transaction.h"
#include "cfdcore/cfdcore_exception.h"
#include "cfdcore/cfdcore_script.h"

using cfd::CoinSelection;
using cfd::CoinSelectionOption;
using cfd::TransactionController;
using cfd::Utxo;
using cfd::UtxoFilter;
using cfd::core::Amount;
using cfd::core::AddressType;
using cfd::core::BlockHash;
using cfd::core::ByteData;
using cfd::core::ByteData256;
using cfd::core::Script;
using cfd::core::Txid;
using cfd::core::CfdException;

#ifndef CFD_DISABLE_ELEMENTS
using cfd::core::ConfidentialAssetId;
#endif  // CFD_DISABLE_ELEMENTS

static CoinSelection exp_selection(false);
static UtxoFilter exp_filter;

static CoinSelectionOption GetBitcoinOption() {
  CoinSelectionOption option;
  option.InitializeTxSizeInfo();
  return option;
}

static std::vector<Utxo> GetBitcoinUtxoList() {
  std::vector<Utxo> utxos;
  {
    Txid txid("7ca81dd22c934747f4f5ab7844178445fe931fb248e0704c062b8f4fbd3d500a");
    struct Utxo utxo;
    memcpy(utxo.txid, txid.GetData().GetBytes().data(), 32);
    utxo.vout = 0;
    utxo.amount = 312500000;
    utxos.push_back(utxo);
  }
  {
    Txid txid("30f71f39d210f7ee291b0969c6935debf11395b0935dca84d30c810a75339a0a");
    struct Utxo utxo;
    memcpy(utxo.txid, txid.GetData().GetBytes().data(), 32);
    utxo.vout = 0;
    utxo.amount = 78125000;
    utxos.push_back(utxo);
  }
  {
    Txid txid("9e1ead91c432889cb478237da974dd1e9009c9e22694fd1e3999c40a1ef59b0a");
    struct Utxo utxo;
    memcpy(utxo.txid, txid.GetData().GetBytes().data(), 32);
    utxo.vout = 0;
    utxo.amount = 1250000000;
    utxos.push_back(utxo);
  }
  {
    Txid txid("8f4af7ee42e62a3d32f25ca56f618fb2f5df3d4c3a9c59e2c3646c5535a3d40a");
    struct Utxo utxo;
    memcpy(utxo.txid, txid.GetData().GetBytes().data(), 32);
    utxo.vout = 0;
    utxo.amount = 39062500;
    utxos.push_back(utxo);
  }
  {
    Txid txid("4d97d0119b90421818bff4ec9033e5199199b53358f56390cb20f8148e76f40a");
    struct Utxo utxo;
    memcpy(utxo.txid, txid.GetData().GetBytes().data(), 32);
    utxo.vout = 0;
    utxo.amount = 156250000;
    utxos.push_back(utxo);
  }
  {
    Txid txid("b9720ed2265a4ced42425bffdb4ef90a473b4106811a802fce53f7c57487fa0b");
    struct Utxo utxo;
    memcpy(utxo.txid, txid.GetData().GetBytes().data(), 32);
    utxo.vout = 0;
    utxo.amount = 2500000000;
    utxos.push_back(utxo);
  }
  {
    Txid txid("0f093988839178ea5895431241cb4400fb31dd7b665a1a93cbd372336c717e0c");
    struct Utxo utxo;
    memcpy(utxo.txid, txid.GetData().GetBytes().data(), 32);
    utxo.vout = 0;
    utxo.amount = 5000000000;
    utxos.push_back(utxo);
  }
  return utxos;
}

// KnapsackSolver-----------------------------------------------------------------
TEST(CoinSelection, KnapsackSolver_targetvalue_0)
{
  Amount target_amount = Amount::CreateBySatoshiAmount(0);
  Amount select_value;
  Amount fee;
  Amount tx_fee = Amount::CreateBySatoshiAmount(1500);
  bool use_bnb = false;
  std::vector<Utxo> ret = exp_selection.SelectCoinsMinConf(
      target_amount, GetBitcoinUtxoList(), exp_filter, GetBitcoinOption(),
      tx_fee, &select_value, &fee, &use_bnb);

  EXPECT_EQ(ret.size(), 0);
  EXPECT_EQ(select_value.GetSatoshiValue(), 0);
  EXPECT_EQ(fee.GetSatoshiValue(), 0);
  EXPECT_FALSE(use_bnb);
}

TEST(CoinSelection, KnapsackSolver_utxos_empty)
{
  Amount target_amount = Amount::CreateBySatoshiAmount(100000000);
  Amount select_value;
  Amount fee;
  Amount tx_fee = Amount::CreateBySatoshiAmount(1500);
  bool use_bnb = false;
  EXPECT_THROW(std::vector<Utxo> ret = exp_selection.SelectCoinsMinConf(
      target_amount, std::vector<Utxo>(), exp_filter, GetBitcoinOption(),
      tx_fee, &select_value, &fee, &use_bnb), CfdException);
}

TEST(CoinSelection, KnapsackSolver_outparameter_nullptr)
{
  Amount target_amount = Amount::CreateBySatoshiAmount(100000000);
  Amount select_value;
  Amount fee;
  Amount tx_fee = Amount::CreateBySatoshiAmount(1500);
  bool use_bnb = false;
  EXPECT_THROW(std::vector<Utxo> ret = exp_selection.SelectCoinsMinConf(
      target_amount, GetBitcoinUtxoList(), exp_filter, GetBitcoinOption(),
      tx_fee, nullptr, &fee, &use_bnb), CfdException);
}

TEST(CoinSelection, KnapsackSolver_match_utxo)
{
  Amount target_amount = Amount::CreateBySatoshiAmount(39062500);
  Amount select_value;
  Amount fee;
  Amount tx_fee = Amount::CreateBySatoshiAmount(1500);
  bool use_bnb = false;
  std::vector<Utxo> ret = exp_selection.SelectCoinsMinConf(
      target_amount, GetBitcoinUtxoList(), exp_filter, GetBitcoinOption(),
      tx_fee, &select_value, &fee, &use_bnb);

  EXPECT_EQ(ret.size(), 1);
  EXPECT_EQ(select_value.GetSatoshiValue(), 39062500);
  EXPECT_EQ(fee.GetSatoshiValue(), 1311520);
  EXPECT_FALSE(use_bnb);
}

TEST(CoinSelection, KnapsackSolver_match_utxo2)
{
  Amount target_amount = Amount::CreateBySatoshiAmount(117187500);
  Amount select_value;
  Amount fee;
  Amount tx_fee = Amount::CreateBySatoshiAmount(1500);
  bool use_bnb = false;
  std::vector<Utxo> ret = exp_selection.SelectCoinsMinConf(
      target_amount, GetBitcoinUtxoList(), exp_filter, GetBitcoinOption(),
      tx_fee, &select_value, &fee, &use_bnb);

  EXPECT_EQ(ret.size(), 2);
  EXPECT_EQ(select_value.GetSatoshiValue(), 117187500);
  EXPECT_EQ(fee.GetSatoshiValue(), 2623040);
  EXPECT_FALSE(use_bnb);
}

TEST(CoinSelection, KnapsackSolver_insufficient_funds)
{
  Amount target_amount = Amount::CreateBySatoshiAmount(9500000000);
  Amount select_value;
  Amount fee;
  Amount tx_fee = Amount::CreateBySatoshiAmount(1500);
  bool use_bnb = false;

  EXPECT_THROW(std::vector<Utxo> ret = exp_selection.SelectCoinsMinConf(
      target_amount, GetBitcoinUtxoList(), exp_filter, GetBitcoinOption(),
      tx_fee, &select_value, &fee, &use_bnb), CfdException);
}

TEST(CoinSelection, KnapsackSolver_lowest_larger)
{
  Amount target_amount = Amount::CreateBySatoshiAmount(120000000);
  Amount select_value;
  Amount fee;
  Amount tx_fee = Amount::CreateBySatoshiAmount(1500);
  bool use_bnb = false;
  std::vector<Utxo> ret = exp_selection.SelectCoinsMinConf(
      target_amount, GetBitcoinUtxoList(), exp_filter, GetBitcoinOption(),
      tx_fee, &select_value, &fee, &use_bnb);

  EXPECT_EQ(ret.size(), 1);
  EXPECT_EQ(select_value.GetSatoshiValue(), 156250000);
  EXPECT_EQ(fee.GetSatoshiValue(), 1311520);
  EXPECT_FALSE(use_bnb);
}

TEST(CoinSelection, KnapsackSolver_ApproximateBestSubset)
{
  Amount target_amount = Amount::CreateBySatoshiAmount(220000000);
  Amount select_value;
  Amount fee;
  Amount tx_fee = Amount::CreateBySatoshiAmount(1500);
  bool use_bnb = false;
  std::vector<Utxo> ret = exp_selection.SelectCoinsMinConf(
      target_amount, GetBitcoinUtxoList(), exp_filter, GetBitcoinOption(),
      tx_fee, &select_value, &fee, &use_bnb);

  EXPECT_EQ(ret.size(), 2);
  EXPECT_EQ(select_value.GetSatoshiValue(), 234375000);
  EXPECT_EQ(fee.GetSatoshiValue(), 2623040);
  EXPECT_FALSE(use_bnb);
}

TEST(CoinSelection, KnapsackSolver_ApproximateBestSubset2)
{
  Amount target_amount = Amount::CreateBySatoshiAmount(460000000);
  Amount select_value;
  Amount fee;
  Amount tx_fee = Amount::CreateBySatoshiAmount(1500);
  bool use_bnb = false;
  std::vector<Utxo> ret = exp_selection.SelectCoinsMinConf(
      target_amount, GetBitcoinUtxoList(), exp_filter, GetBitcoinOption(),
      tx_fee, &select_value, &fee, &use_bnb);

  EXPECT_EQ(ret.size(), 2);
  EXPECT_EQ(select_value.GetSatoshiValue(), 468750000);
  EXPECT_EQ(fee.GetSatoshiValue(), 2623040);
  EXPECT_FALSE(use_bnb);
}

// SelectCoinsBnB-----------------------------------------------------------------
struct TestUtxoCoinVector {
  std::string txid;
  uint32_t vout;
  uint64_t amount;
  std::string descriptor;
};

/*
  const reqJson = {
    utxos: [{
      txid: 'eed524525ec8f94348f3d65661501b293b936e8fff4f2ff9ee70818f17367efe',
      vout: 0,
      amount: 155062500,
      descriptor: 'sh(wpkh([ef735203/0\'/0\'/7\']022c2409fbf657ba25d97bb3dab5426d20677b774d4fc7bd3bfac27ff96ada3dd1))#4z2vy08x',
    }, {
      txid: 'eed524525ec8f94348f3d65661501b293b936e8fff4f2ff9ee70818f17367efe',
      vout: 0,
      amount: 85062500,
      descriptor: 'sh(wpkh([ef735203/0\'/0\'/7\']022c2409fbf657ba25d97bb3dab5426d20677b774d4fc7bd3bfac27ff96ada3dd1))#4z2vy08x',
    }, {
      txid: 'efd524525ec8f94348f3d65661501b293b936e8fff4f2ff9ee70818f17367efe',
      vout: 0,
      amount: 61062500,
      descriptor: 'sh(wpkh([ef735203/0\'/0\'/7\']022c2409fbf657ba25d97bb3dab5426d20677b774d4fc7bd3bfac27ff96ada3dd1))#4z2vy08x',
    }, {
      txid: 'ead524525ec8f94348f3d65661501b293b936e8fff4f2ff9ee70818f17367efe',
      vout: 0,
      amount: 39062500,
      descriptor: 'sh(wpkh([ef735203/0\'/0\'/7\']022c2409fbf657ba25d97bb3dab5426d20677b774d4fc7bd3bfac27ff96ada3dd1))#4z2vy08x',
    }, {
      txid: 'eac524525ec8f94348f3d65661501b293b936e8fff4f2ff9ee70818f17367efe',
      vout: 0,
      amount: 15675000,
      descriptor: 'sh(wpkh([ef735203/0\'/0\'/7\']022c2409fbf657ba25d97bb3dab5426d20677b774d4fc7bd3bfac27ff96ada3dd1))#4z2vy08x',
    }, {
      txid: '0f59594cfecf8fe1733521e29736352935711f34cd958f34df4a031858f6ecfd',
      vout: 0,
      amount: 14938590,
      descriptor: 'sh(wpkh([ef735203/0\'/0\'/4\']0231c043ae680664a2c5df38cf0d8eab29f1b61ce93855040c613b2f41f7c036af))#pezpv0hm',
    }],
    targetAmount: 100000000,
    feeInfo: {
      feeRate: 2,
      transaction: '02000000000100e1f5050000000017a914e37a3603a4d392f9ecb68b32eac6ba19adc4968f8700000000',
      isElements: false,
    },
  };
*/

static const std::vector<TestUtxoCoinVector> kExtCoinSelectTestVector = {
  {
    "",
    0,
    155062500,
    "sh(wpkh([ef735203/0'/0'/7']022c2409fbf657ba25d97bb3dab5426d20677b774d4fc7bd3bfac27ff96ada3dd1))#4z2vy08x"
  },
  {
    "",
    0,
    85062500,
    "sh(wpkh([ef735203/0'/0'/7']022c2409fbf657ba25d97bb3dab5426d20677b774d4fc7bd3bfac27ff96ada3dd1))#4z2vy08x"
  },
  {
    "",
    0,
    39062500,
    "sh(wpkh([ef735203/0'/0'/7']022c2409fbf657ba25d97bb3dab5426d20677b774d4fc7bd3bfac27ff96ada3dd1))#4z2vy08x"
  },
  {
    "",
    0,
    61062500,
    "sh(wpkh([ef735203/0'/0'/7']022c2409fbf657ba25d97bb3dab5426d20677b774d4fc7bd3bfac27ff96ada3dd1))#4z2vy08x"
  },
  {
    "",
    0,
    15675000,
    "sh(wpkh([ef735203/0'/0'/7']022c2409fbf657ba25d97bb3dab5426d20677b774d4fc7bd3bfac27ff96ada3dd1))#4z2vy08x"
  },
  {
    "",
    0,
    14938590,
    "sh(wpkh([ef735203/0'/0'/7']022c2409fbf657ba25d97bb3dab5426d20677b774d4fc7bd3bfac27ff96ada3dd1))#4z2vy08x"
  }
};

TEST(CoinSelection, SelectCoinsMinConf_SelectCoinsBnB)
{
  cfd::core::CfdCoreHandle handle = nullptr;
  cfd::core::Initialize(&handle);
  CoinSelection coin_select(true);

  Amount target_value = Amount::CreateBySatoshiAmount(99998500);
  std::vector<Utxo> utxos;
  UtxoFilter filter;
  CoinSelectionOption option_params;
  Amount select_value;
  Amount fee_value;
  std::vector<Utxo> select_utxos;
  Amount tx_fee = Amount::CreateBySatoshiAmount(1500);
  bool use_bnb = false;

  utxos.resize(kExtCoinSelectTestVector.size());
  std::vector<Utxo>::iterator ite = utxos.begin();
  for (const auto& test_data : kExtCoinSelectTestVector) {
    Txid txid;
    if (!test_data.txid.empty()) {
      txid = Txid(test_data.txid);
    }
    CoinSelection::ConvertToUtxo(
        txid, test_data.vout, test_data.descriptor,
        Amount::CreateBySatoshiAmount(test_data.amount), "", nullptr,
        &(*ite));
    ++ite;
  }

  option_params.InitializeTxSizeInfo();
  option_params.SetEffectiveFeeBaserate(2);

  EXPECT_NO_THROW((select_utxos = coin_select.SelectCoinsMinConf(target_value, utxos,
      filter, option_params, tx_fee, &select_value, &fee_value, &use_bnb)));
  EXPECT_EQ(select_utxos.size(), 2);
  EXPECT_EQ(select_value.GetSatoshiValue(), static_cast<int64_t>(100001090));
  EXPECT_EQ(fee_value.GetSatoshiValue(), static_cast<int64_t>(360));
  if (select_utxos.size() == 2) {
    EXPECT_EQ(select_utxos[0].amount, static_cast<int64_t>(85062500));
    EXPECT_EQ(select_utxos[1].amount, static_cast<int64_t>(14938590));
  }
  EXPECT_TRUE(use_bnb);
}

TEST(CoinSelection, SelectCoinsMinConf_SelectCoinsBnB_single)
{
  cfd::core::CfdCoreHandle handle = nullptr;
  cfd::core::Initialize(&handle);
  CoinSelection coin_select(true);

  Amount target_value = Amount::CreateBySatoshiAmount(155060800);
  std::vector<Utxo> utxos;
  UtxoFilter filter;
  CoinSelectionOption option_params;
  Amount select_value;
  Amount fee_value;
  std::vector<Utxo> select_utxos;
  Amount tx_fee = Amount::CreateBySatoshiAmount(1500);
  bool use_bnb = false;

  utxos.resize(kExtCoinSelectTestVector.size());
  std::vector<Utxo>::iterator ite = utxos.begin();
  for (const auto& test_data : kExtCoinSelectTestVector) {
    Txid txid;
    if (!test_data.txid.empty()) {
      txid = Txid(test_data.txid);
    }
    CoinSelection::ConvertToUtxo(
        txid, test_data.vout, test_data.descriptor,
        Amount::CreateBySatoshiAmount(test_data.amount), "", nullptr,
        &(*ite));
    ++ite;
  }

  option_params.InitializeTxSizeInfo();
  option_params.SetEffectiveFeeBaserate(2);

  EXPECT_NO_THROW((select_utxos = coin_select.SelectCoinsMinConf(target_value, utxos,
      filter, option_params, tx_fee, &select_value, &fee_value, &use_bnb)));
  EXPECT_EQ(select_utxos.size(), 1);
  EXPECT_EQ(select_value.GetSatoshiValue(), static_cast<int64_t>(155062500));
  EXPECT_EQ(fee_value.GetSatoshiValue(), static_cast<int64_t>(180));
  if (select_utxos.size() == 1) {
    EXPECT_EQ(select_utxos[0].amount, static_cast<int64_t>(155062500));
  }
  EXPECT_TRUE(use_bnb);
}

TEST(CoinSelection, SelectCoinsMinConf_SelectCoinsBnB_empty)
{
  // BnB fail. (use knapsack)
  CoinSelection coin_select(true);

  Amount target_value = Amount::CreateBySatoshiAmount(114060000);
  std::vector<Utxo> utxos;
  UtxoFilter filter;
  CoinSelectionOption option_params;
  Amount select_value;
  Amount fee_value;
  std::vector<Utxo> select_utxos;
  Amount tx_fee = Amount::CreateBySatoshiAmount(1500);
  bool use_bnb = false;

  utxos.resize(kExtCoinSelectTestVector.size());
  std::vector<Utxo>::iterator ite = utxos.begin();
  for (const auto& test_data : kExtCoinSelectTestVector) {
    Txid txid;
    if (!test_data.txid.empty()) {
      txid = Txid(test_data.txid);
    }
    CoinSelection::ConvertToUtxo(
        txid, test_data.vout, test_data.descriptor,
        Amount::CreateBySatoshiAmount(test_data.amount), "", nullptr,
        &(*ite));
    ++ite;
  }

  option_params.InitializeTxSizeInfo();
  option_params.SetEffectiveFeeBaserate(1);

  EXPECT_NO_THROW((select_utxos = coin_select.SelectCoinsMinConf(target_value, utxos,
      filter, option_params, tx_fee, &select_value, &fee_value, &use_bnb)));
  EXPECT_EQ(select_utxos.size(), 3);
  EXPECT_EQ(select_value.GetSatoshiValue(), static_cast<int64_t>(115063590));
  EXPECT_EQ(fee_value.GetSatoshiValue(), static_cast<int64_t>(270));
  if (select_utxos.size() == 3) {
    EXPECT_EQ(select_utxos[0].amount, static_cast<int64_t>(61062500));
    EXPECT_EQ(select_utxos[1].amount, static_cast<int64_t>(39062500));
    EXPECT_EQ(select_utxos[2].amount, static_cast<int64_t>(14938590));
  }
  EXPECT_FALSE(use_bnb);
}


TEST(CoinSelection, SelectCoinsMinConf_SelectCoinsBnB_low_amount)
{
  CoinSelection coin_select(true);

  Amount target_value = Amount::CreateBySatoshiAmount(500000000);
  std::vector<Utxo> utxos;
  UtxoFilter filter;
  CoinSelectionOption option_params;
  Amount select_value;
  Amount fee_value;
  std::vector<Utxo> select_utxos;
  Amount tx_fee = Amount::CreateBySatoshiAmount(1500);
  bool use_bnb = false;

  utxos.resize(kExtCoinSelectTestVector.size());
  std::vector<Utxo>::iterator ite = utxos.begin();
  for (const auto& test_data : kExtCoinSelectTestVector) {
    Txid txid;
    if (!test_data.txid.empty()) {
      txid = Txid(test_data.txid);
    }
    CoinSelection::ConvertToUtxo(
        txid, test_data.vout, test_data.descriptor,
        Amount::CreateBySatoshiAmount(test_data.amount), "", nullptr,
        &(*ite));
    ++ite;
  }

  option_params.InitializeTxSizeInfo();
  option_params.SetEffectiveFeeBaserate(1);

  EXPECT_THROW((select_utxos = coin_select.SelectCoinsMinConf(target_value, utxos,
      filter, option_params, tx_fee, &select_value, &fee_value, &use_bnb)), CfdException);
}


TEST(CoinSelection, SelectCoinsMinConf_SelectCoinsBnB_effective_value_zero)
{
  CoinSelection coin_select(true);

  Amount target_value = Amount::CreateBySatoshiAmount(5);
  std::vector<Utxo> utxos;
  UtxoFilter filter;
  CoinSelectionOption option_params;
  Amount select_value;
  Amount fee_value;
  std::vector<Utxo> select_utxos;
  Amount tx_fee = Amount::CreateBySatoshiAmount(1500);
  bool use_bnb = false;

  utxos.resize(kExtCoinSelectTestVector.size());
  std::vector<Utxo>::iterator ite = utxos.begin();
  for (const auto& test_data : kExtCoinSelectTestVector) {
    Txid txid;
    if (!test_data.txid.empty()) {
      txid = Txid(test_data.txid);
    }
    CoinSelection::ConvertToUtxo(
        txid, test_data.vout, test_data.descriptor,
        Amount::CreateBySatoshiAmount(10), "", nullptr,
        &(*ite));
    ++ite;
  }

  option_params.InitializeTxSizeInfo();
  option_params.SetEffectiveFeeBaserate(1);

  EXPECT_THROW((select_utxos = coin_select.SelectCoinsMinConf(target_value, utxos,
      filter, option_params, tx_fee, &select_value, &fee_value, &use_bnb)), CfdException);
}

TEST(CoinSelection, Constructor)
{
  CoinSelection coin_select1;
  CoinSelection coin_select2(false);
}

TEST(CoinSelection, ConvertToUtxo)
{
  uint64_t block_height = 1;
  BlockHash block_hash("1234567890123456789012345678901234567890123456789012345678901456");
  Txid txid("0034567890123456789012345678901234567890123456789012345678901456");
  uint32_t vout = 2;
  Script locking_script("00141234567890123456789012345678901234567890");
  std::string output_descriptor("wpkh([ef735203/0'/0'/7']022c2409fbf657ba25d97bb3dab5426d20677b774d4fc7bd3bfac27ff96ada3dd1)#4z2vy08x");
  Amount amount = Amount::CreateBySatoshiAmount(20000);
  void* binary_data = &vout;
  Utxo utxo = {};
  // std::vector<uint8_t> script_bytes;

  EXPECT_NO_THROW((CoinSelection::ConvertToUtxo(
    block_height, block_hash, txid, vout, locking_script,
    output_descriptor, amount, binary_data, &utxo)));

  if (utxo.script_length != 0) {
    // script_bytes.resize(utxo.script_length);
    // memcpy(script_bytes.data(), utxo.locking_script, utxo.script_length);
    EXPECT_STREQ(ByteData(std::vector<uint8_t>(std::begin(utxo.locking_script), std::begin(utxo.locking_script) + utxo.script_length)).GetHex().c_str(), locking_script.GetHex().c_str());
  }
  EXPECT_EQ(utxo.block_height, block_height);
  EXPECT_STREQ(BlockHash(ByteData256(std::vector<uint8_t>(std::begin(utxo.block_hash), std::end(utxo.block_hash)))).GetHex().c_str(), block_hash.GetHex().c_str());
  EXPECT_STREQ(Txid(ByteData256(std::vector<uint8_t>(std::begin(utxo.txid), std::end(utxo.txid)))).GetHex().c_str(), txid.GetHex().c_str());
  EXPECT_EQ(utxo.vout, vout);
  EXPECT_EQ(utxo.script_length, locking_script.GetData().GetDataSize());
  EXPECT_EQ(utxo.address_type, static_cast<uint16_t>(AddressType::kP2wpkhAddress));
  EXPECT_EQ(utxo.witness_size_max, static_cast<uint16_t>(108));
  EXPECT_EQ(utxo.uscript_size_max, static_cast<uint16_t>(0));
  EXPECT_EQ(utxo.amount, amount.GetSatoshiValue());
  EXPECT_EQ(utxo.binary_data, binary_data);

#ifndef CFD_DISABLE_ELEMENTS
  ConfidentialAssetId asset("9934567890123456789012345678901234567890123456789012345678901456");
  memset(&utxo, 0, sizeof(utxo));
  EXPECT_NO_THROW((CoinSelection::ConvertToUtxo(
    block_height, block_hash, txid, vout, locking_script,
    output_descriptor, amount, asset, binary_data, &utxo)));

  if (utxo.script_length != 0) {
    // script_bytes.resize(utxo.script_length);
    // memcpy(script_bytes.data(), utxo.locking_script, utxo.script_length);
    EXPECT_STREQ(ByteData(std::vector<uint8_t>(std::begin(utxo.locking_script), std::begin(utxo.locking_script) + utxo.script_length)).GetHex().c_str(), locking_script.GetHex().c_str());
  }
  EXPECT_EQ(utxo.block_height, block_height);
  EXPECT_STREQ(BlockHash(ByteData256(std::vector<uint8_t>(std::begin(utxo.block_hash), std::end(utxo.block_hash)))).GetHex().c_str(), block_hash.GetHex().c_str());
  EXPECT_STREQ(Txid(ByteData256(std::vector<uint8_t>(std::begin(utxo.txid), std::end(utxo.txid)))).GetHex().c_str(), txid.GetHex().c_str());
  EXPECT_EQ(utxo.vout, vout);
  EXPECT_EQ(utxo.script_length, locking_script.GetData().GetDataSize());
  EXPECT_EQ(utxo.address_type, static_cast<uint16_t>(AddressType::kP2wpkhAddress));
  EXPECT_EQ(utxo.witness_size_max, static_cast<uint16_t>(108));
  EXPECT_EQ(utxo.uscript_size_max, static_cast<uint16_t>(0));
  EXPECT_EQ(utxo.amount, amount.GetSatoshiValue());
  EXPECT_EQ(utxo.binary_data, binary_data);
  EXPECT_STREQ(ConfidentialAssetId(ByteData(std::vector<uint8_t>(std::begin(utxo.asset), std::end(utxo.asset)))).GetHex().c_str(), asset.GetHex().c_str());
#endif                // CFD_DISABLE_ELEMENTS
}
