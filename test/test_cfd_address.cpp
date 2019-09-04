#include "gtest/gtest.h"
#include <vector>

#include "cfd/cfd_common.h"
#include "cfd/cfd_address.h"

// https://qiita.com/yohm/items/477bac065f4b772127c7

// The main function are using gtest's main().

// TEST(test_suite_name, test_name)

using cfdcore::Address;
using cfdcore::NetType;
using cfdcore::Pubkey;
using cfdcore::Script;
using cfdcore::WitnessVersion;
using cfd::AddressUtil;

TEST(AddressUtil, CreateP2pkhAddress)
{
  Pubkey pubkey = Pubkey("027592aab5d43618dda13fba71e3993cd7517a712d3da49664c06ee1bd3d1f70af");
  Address addr = AddressUtil::CreateP2pkhAddress(pubkey, NetType::kMainnet);
  EXPECT_STREQ(addr.GetAddress().c_str(), "1ELuNB5fLNUcrLzb93oJDPmjxjnsVwhNHn");
}
