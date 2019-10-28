#include "gtest/gtest.h"
#include <vector>

#include "cfd/cfd_common.h"
#include "cfd/cfd_elements_address.h"
#include "cfdcore/cfdcore_exception.h"
#include "cfdcore/cfdcore_elements_address.h"


// https://qiita.com/yohm/items/477bac065f4b772127c7

// The main function are using gtest's main().

// TEST(test_suite_name, test_name)

using cfd::core::Address;
using cfd::core::AddressType;
using cfd::core::ByteData;
using cfd::core::ByteData160;
using cfd::core::CfdException;
using cfd::core::ConfidentialKey;
using cfd::core::GetElementsAddressFormatList;
using cfd::core::GetBitcoinAddressFormatList;
using cfd::core::NetType;
using cfd::core::Pubkey;
using cfd::core::Script;
using cfd::core::WitnessVersion;
using cfd::ElementsAddressFactory;
using cfd::core::ElementsConfidentialAddress;

TEST(ElementsAddressFactory, Constructor)
{
  EXPECT_NO_THROW(ElementsAddressFactory factory);
}

TEST(ElementsAddressFactory, Constructor_type)
{
  EXPECT_NO_THROW(ElementsAddressFactory factory(NetType::kLiquidV1));
  EXPECT_NO_THROW(ElementsAddressFactory factory(NetType::kElementsRegtest));
}

TEST(ElementsAddressFactory, Constructor_type_prefix)
{
  EXPECT_NO_THROW(ElementsAddressFactory factory(NetType::kMainnet, GetBitcoinAddressFormatList()));
  EXPECT_NO_THROW(ElementsAddressFactory factory(NetType::kTestnet, GetBitcoinAddressFormatList()));
  EXPECT_NO_THROW(ElementsAddressFactory factory(NetType::kRegtest, GetBitcoinAddressFormatList()));
  EXPECT_NO_THROW(ElementsAddressFactory factory(NetType::kLiquidV1, GetElementsAddressFormatList()));
  EXPECT_NO_THROW(ElementsAddressFactory factory(NetType::kElementsRegtest, GetElementsAddressFormatList()));
}

TEST(ElementsAddressFactory, Constructor_type_witver)
{
  EXPECT_NO_THROW(ElementsAddressFactory factory(NetType::kLiquidV1, WitnessVersion::kVersion0));
  EXPECT_NO_THROW(ElementsAddressFactory factory(NetType::kElementsRegtest, WitnessVersion::kVersionNone));
}


TEST(ElementsAddressFactory, Constructor_type_witver_prefix)
{
  EXPECT_NO_THROW(ElementsAddressFactory factory(NetType::kMainnet, WitnessVersion::kVersion0, GetBitcoinAddressFormatList()));
  EXPECT_NO_THROW(ElementsAddressFactory factory(NetType::kTestnet, WitnessVersion::kVersion1, GetBitcoinAddressFormatList()));
  EXPECT_NO_THROW(ElementsAddressFactory factory(NetType::kRegtest, WitnessVersion::kVersionNone, GetBitcoinAddressFormatList()));
  EXPECT_NO_THROW(ElementsAddressFactory factory(NetType::kLiquidV1, WitnessVersion::kVersion0, GetElementsAddressFormatList()));
  EXPECT_NO_THROW(ElementsAddressFactory factory(NetType::kElementsRegtest, WitnessVersion::kVersionNone, GetElementsAddressFormatList()));
}

TEST(ElementsAddressFactory, GetConfidentialAddress)
{
  ConfidentialKey pubkey = ConfidentialKey("027592aab5d43618dda13fba71e3993cd7517a712d3da49664c06ee1bd3d1f70af");
  {
    Address address("ex1q7s3nr3qcaaz30wnyfttwnlyedqddgwfhclhncc", GetElementsAddressFormatList());
    ElementsConfidentialAddress conf_address = ElementsAddressFactory::GetConfidentialAddress(address, pubkey);

    EXPECT_STREQ(conf_address.GetConfidentialKey().GetHex().c_str(), pubkey.GetHex().c_str());
    EXPECT_STREQ(conf_address.GetUnblindedAddress().GetAddress().c_str(), address.GetAddress().c_str());
  }
  {
    Address address("XRpicZNrFZumBMhRV5BSYW28pGX7JyY1ua", GetElementsAddressFormatList());
    ElementsConfidentialAddress conf_address = ElementsAddressFactory::GetConfidentialAddress(address, pubkey);

    EXPECT_STREQ(conf_address.GetConfidentialKey().GetHex().c_str(), pubkey.GetHex().c_str());
    EXPECT_STREQ(conf_address.GetUnblindedAddress().GetAddress().c_str(), address.GetAddress().c_str());
  }

  // error
  Address err_address("bcrt1qjylqhy5pm2ck75ppqxk5ue2swsuk7dx9mmfljp");
  EXPECT_THROW(ElementsAddressFactory::GetConfidentialAddress(err_address, pubkey), CfdException);
  Address err_address2("2NDBYYaMywkXcQ4cupdKQdoyrm9iHHSLtdy");
  EXPECT_THROW(ElementsAddressFactory::GetConfidentialAddress(err_address2, pubkey), CfdException);
}

TEST(ElementsAddressFactory, CreatePegInAddress1)
{
  Pubkey pubkey = Pubkey("027592aab5d43618dda13fba71e3993cd7517a712d3da49664c06ee1bd3d1f70af");
  Script fedpegscript("522102baae8e066e4f2a1da4b731017697bb8fcacc60e4569f3ec27bc31cf3fb13246221026bccd050e8ecf7a702bc9fb63205cfdf278a22ba8b1f1d3ca3d8e5b38465a9702103430d354b89d1fbe43eb54ea138a4aee1076e4c54f4c805f62f9cee965351a1d053ae");
  {
    ElementsAddressFactory factory(NetType::kLiquidV1);
    EXPECT_NO_THROW(Address address = factory.CreatePegInAddress(AddressType::kP2wpkhAddress, pubkey, fedpegscript));
    // error
    EXPECT_THROW(factory.CreatePegInAddress(AddressType::kP2wpkhAddress, pubkey, Script()), CfdException);
  }
  {
    ElementsAddressFactory factory(NetType::kElementsRegtest);
    EXPECT_NO_THROW(Address address = factory.CreatePegInAddress(AddressType::kP2wpkhAddress, pubkey, fedpegscript));
    // error
    EXPECT_THROW(factory.CreatePegInAddress(AddressType::kP2wpkhAddress, pubkey, Script()), CfdException);
  }
}

TEST(ElementsAddressFactory, CreatePegInAddress2)
{
  Script claim_script("76a914c3bec9f6c51c3d1f0fd00c54fe41b5d8014ed8eb88ac");
  Script fedpegscript("522102baae8e066e4f2a1da4b731017697bb8fcacc60e4569f3ec27bc31cf3fb13246221026bccd050e8ecf7a702bc9fb63205cfdf278a22ba8b1f1d3ca3d8e5b38465a9702103430d354b89d1fbe43eb54ea138a4aee1076e4c54f4c805f62f9cee965351a1d053ae");
  {
    ElementsAddressFactory factory(NetType::kLiquidV1);
    EXPECT_NO_THROW(Address address = factory.CreatePegInAddress(AddressType::kP2wpkhAddress, claim_script, fedpegscript));
    // error
    EXPECT_THROW(factory.CreatePegInAddress(AddressType::kP2wpkhAddress, claim_script, Script()), CfdException);
    EXPECT_THROW(factory.CreatePegInAddress(AddressType::kP2wpkhAddress, Script(), fedpegscript), CfdException);
  }
  {
    ElementsAddressFactory factory(NetType::kElementsRegtest);
    EXPECT_NO_THROW(Address address = factory.CreatePegInAddress(AddressType::kP2wpkhAddress, claim_script, fedpegscript));
    // error
    EXPECT_THROW(factory.CreatePegInAddress(AddressType::kP2wpkhAddress, claim_script, Script()), CfdException);
    EXPECT_THROW(factory.CreatePegInAddress(AddressType::kP2wpkhAddress, Script(), fedpegscript), CfdException);
  }
}

TEST(ElementsAddressFactory, CreatePegInAddress3)
{
  Script tweak_fedpegscript("522102baae8e066e4f2a1da4b731017697bb8fcacc60e4569f3ec27bc31cf3fb13246221026bccd050e8ecf7a702bc9fb63205cfdf278a22ba8b1f1d3ca3d8e5b38465a9702103430d354b89d1fbe43eb54ea138a4aee1076e4c54f4c805f62f9cee965351a1d053ae");
  {
    ElementsAddressFactory factory(NetType::kLiquidV1);
    EXPECT_NO_THROW(Address address = factory.CreatePegInAddress(AddressType::kP2wpkhAddress, tweak_fedpegscript));
    // error
    EXPECT_THROW(factory.CreatePegInAddress(AddressType::kP2wpkhAddress, Script()), CfdException);
  }
  {
    ElementsAddressFactory factory(NetType::kElementsRegtest);
    EXPECT_NO_THROW(Address address = factory.CreatePegInAddress(AddressType::kP2wpkhAddress, tweak_fedpegscript));
    // error
    EXPECT_THROW(factory.CreatePegInAddress(AddressType::kP2wpkhAddress, Script()), CfdException);
  }
}
