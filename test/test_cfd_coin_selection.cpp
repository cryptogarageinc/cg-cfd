#include "gtest/gtest.h"
#include <vector>

#include "cfd/cfd_common.h"
#include "cfd/cfd_utxo.h"
#include "cfdcore/cfdcore_exception.h"

using cfd::CoinSelection;
using cfd::CoinSelectionOption;
using cfd::TransactionController;
using cfd::Utxo;
using cfd::UtxoFilter;
using cfd::core::Amount;
using cfd::core::Txid;
using cfd::core::CfdException;


static CoinSelection exp_selection(false);
static UtxoFilter exp_filter;

static CoinSelectionOption GetBitcoinOption() {
  TransactionController txc("02000000000100e1f5050000000017a914e37a3603a4d392f9ecb68b32eac6ba19adc4968f8700000000");
  CoinSelectionOption option;
  option.InitializeTxSize(txc);
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
  std::vector<Utxo> ret = exp_selection.SelectCoinsMinConf(
      target_amount, GetBitcoinUtxoList(), exp_filter, GetBitcoinOption(), &select_value, &fee);

  EXPECT_EQ(ret.size(), 0);
  EXPECT_EQ(select_value.GetSatoshiValue(), 0);
  EXPECT_EQ(fee.GetSatoshiValue(), 0);
}

TEST(CoinSelection, KnapsackSolver_utxos_empty)
{
  Amount target_amount = Amount::CreateBySatoshiAmount(100000000);
  Amount select_value;
  Amount fee;
  EXPECT_THROW(std::vector<Utxo> ret = exp_selection.SelectCoinsMinConf(
      target_amount, std::vector<Utxo>(), exp_filter, GetBitcoinOption(), &select_value, &fee), CfdException);
}

TEST(CoinSelection, KnapsackSolver_outparameter_nullptr)
{
  Amount target_amount = Amount::CreateBySatoshiAmount(100000000);
  Amount select_value;
  Amount fee;
  EXPECT_THROW(std::vector<Utxo> ret = exp_selection.SelectCoinsMinConf(
      target_amount, GetBitcoinUtxoList(), exp_filter, GetBitcoinOption(), nullptr, &fee), CfdException);
}

TEST(CoinSelection, KnapsackSolver_match_utxo)
{
  Amount target_amount = Amount::CreateBySatoshiAmount(39062500);
  Amount select_value;
  Amount fee;
  std::vector<Utxo> ret = exp_selection.SelectCoinsMinConf(
      target_amount, GetBitcoinUtxoList(), exp_filter, GetBitcoinOption(), &select_value, &fee);

  EXPECT_EQ(ret.size(), 1);
  EXPECT_EQ(select_value.GetSatoshiValue(), 39062500);
  EXPECT_EQ(fee.GetSatoshiValue(), 0);
}

TEST(CoinSelection, KnapsackSolver_match_utxo2)
{
  Amount target_amount = Amount::CreateBySatoshiAmount(117187500);
  Amount select_value;
  Amount fee;
  std::vector<Utxo> ret = exp_selection.SelectCoinsMinConf(
      target_amount, GetBitcoinUtxoList(), exp_filter, GetBitcoinOption(), &select_value, &fee);

  EXPECT_EQ(ret.size(), 2);
  EXPECT_EQ(select_value.GetSatoshiValue(), 117187500);
  EXPECT_EQ(fee.GetSatoshiValue(), 0);
}

TEST(CoinSelection, KnapsackSolver_insufficient_funds)
{
  Amount target_amount = Amount::CreateBySatoshiAmount(9500000000);
  Amount select_value;
  Amount fee;

  EXPECT_THROW(std::vector<Utxo> ret = exp_selection.SelectCoinsMinConf(
      target_amount, GetBitcoinUtxoList(), exp_filter, GetBitcoinOption(), &select_value, &fee), CfdException);
}

TEST(CoinSelection, KnapsackSolver_lowest_larger)
{
  Amount target_amount = Amount::CreateBySatoshiAmount(120000000);
  Amount select_value;
  Amount fee;
  std::vector<Utxo> ret = exp_selection.SelectCoinsMinConf(
      target_amount, GetBitcoinUtxoList(), exp_filter, GetBitcoinOption(), &select_value, &fee);

  EXPECT_EQ(ret.size(), 1);
  EXPECT_EQ(select_value.GetSatoshiValue(), 156250000);
  EXPECT_EQ(fee.GetSatoshiValue(), 36250000);
}

TEST(CoinSelection, KnapsackSolver_ApproximateBestSubset)
{
  Amount target_amount = Amount::CreateBySatoshiAmount(220000000);
  Amount select_value;
  Amount fee;
  std::vector<Utxo> ret = exp_selection.SelectCoinsMinConf(
      target_amount, GetBitcoinUtxoList(), exp_filter, GetBitcoinOption(), &select_value, &fee);

  EXPECT_EQ(ret.size(), 2);
  EXPECT_EQ(select_value.GetSatoshiValue(), 234375000);
  EXPECT_EQ(fee.GetSatoshiValue(), 14375000);
}

TEST(CoinSelection, KnapsackSolver_ApproximateBestSubset2)
{
  Amount target_amount = Amount::CreateBySatoshiAmount(460000000);
  Amount select_value;
  Amount fee;
  std::vector<Utxo> ret = exp_selection.SelectCoinsMinConf(
      target_amount, GetBitcoinUtxoList(), exp_filter, GetBitcoinOption(), &select_value, &fee);

  EXPECT_EQ(ret.size(), 2);
  EXPECT_EQ(select_value.GetSatoshiValue(), 468750000);
  EXPECT_EQ(fee.GetSatoshiValue(), 8750000);
}
