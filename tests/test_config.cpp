#include "Log/Log.h"
#include "Util/Config.h"
#include "Util/CustomConfigVar.h"
#include <gtest/gtest.h>

solar::ConfigVar<int>::ptr g_intValueConfig =
    solar::Config::Lookup("system.port", (int)8080, "system port");

solar::ConfigVar<double>::ptr g_doubleValueConfig =
    solar::Config::Lookup("system.dobule", (double)3.14, "system double");

solar::ConfigVar<std::vector<int>>::ptr g_intVecConfig = solar::Config::Lookup(
    "system.int_vec", std::vector<int>{1, 2}, "system int vec");

solar::ConfigVar<std::list<int>>::ptr g_intListConfig = solar::Config::Lookup(
    "system.int_list", std::list<int>{1, 2}, "system int list");

solar::ConfigVar<std::set<int>>::ptr g_intSetConfig = solar::Config::Lookup(
    "system.int_set", std::set<int>{1, 2, 2}, "system int set");

solar::ConfigVar<std::map<std::string, int>>::ptr g_intMapConfig =
    solar::Config::Lookup("system.int_map",
                          std::map<std::string, int>{{"k", 1}},
                          "system int map");

solar::ConfigVar<std::unordered_set<int>>::ptr g_intUsetConfig =
    solar::Config::Lookup("system.int_uset", std::unordered_set<int>{1, 2, 2},
                          "system int uset");

solar::ConfigVar<std::unordered_map<std::string, int>>::ptr g_intUmapConfig =
    solar::Config::Lookup("system.int_umap",
                          std::unordered_map<std::string, int>{{"k", 1}},
                          "system int umap");

solar::ConfigVar<std::unordered_map<std::string, std::vector<int>>>::ptr
    g_strIntVecUmapConfig = solar::Config::Lookup(
        "system.str_int_vec_umap",
        std::unordered_map<std::string, std::vector<int>>{{"k", {1, 2, 3}}},
        "system str int vec umap");

TEST(ConfigTest, BuiltinTypes) {
    EXPECT_EQ(g_intValueConfig->getValue(), 8080);
    solar::ConfigVar<float>::ptr value =
        solar::Config::Lookup("system.port", (float)8080, "system port");
    EXPECT_FALSE(value);
    EXPECT_EQ(g_doubleValueConfig->getValue(), 3.14);
}

TEST(ConfigTest, ComplexTypes) {
    std::vector<int> vec{1, 2};
    EXPECT_EQ(g_intVecConfig->getValue(), vec);

    std::list<int> list{1, 2};
    EXPECT_EQ(g_intListConfig->getValue(), list);

    std::set<int> set{1, 2};
    EXPECT_EQ(g_intSetConfig->getValue(), set);

    std::map<std::string, int> map{{"k", 1}};
    EXPECT_EQ(g_intMapConfig->getValue(), map);

    std::unordered_set<int> uset{1, 2};
    EXPECT_EQ(g_intUsetConfig->getValue(), uset);

    std::unordered_map<std::string, int> umap{{"k", 1}};
    EXPECT_EQ(g_intUmapConfig->getValue(), umap);

    std::unordered_map<std::string, std::vector<int>> strIntVecMap{
        {"k", {1, 2, 3}}};
    EXPECT_EQ(g_strIntVecUmapConfig->getValue(), strIntVecMap);
}

TEST(ConfigTest, LoadFromYAML) {
    YAML::Node root = YAML::LoadFile("test.yml");
    solar::Config::LoadFromYaml(root);
    EXPECT_EQ(g_intValueConfig->getValue(), 9999);

    std::vector<int> vec{1, 2};
    EXPECT_EQ(g_intVecConfig->getValue(), vec);

    std::list<int> list{3, 4};
    EXPECT_EQ(g_intListConfig->getValue(), list);

    std::set<int> set{5, 6};
    EXPECT_EQ(g_intSetConfig->getValue(), set);

    std::map<std::string, int> map{{"k", 7}, {"k1", 6}};
    EXPECT_EQ(g_intMapConfig->getValue(), map);

    std::unordered_set<int> uset{8, 9};
    EXPECT_EQ(g_intUsetConfig->getValue(), uset);

    std::unordered_map<std::string, int> umap{{"k", 10}};
    EXPECT_EQ(g_intUmapConfig->getValue(), umap);

    std::unordered_map<std::string, std::vector<int>> strIntVecUmap{
        {"k", {4, 5, 6}}};
    EXPECT_EQ(g_strIntVecUmapConfig->getValue(), strIntVecUmap);
}

struct Person : public solar::CustomConfigVar {
    std::string name;
    int age;
    bool sex;
    bool operator==(const Person &other) const {
        return name == other.name && age == other.age && sex == other.sex;
    }
    bool fromYaml(const std::string &str) override {
        return solar::FromYamlString(str, "name", name, "age", age, "sex", sex);
    }
    std::string toYaml() const override {
        return solar::ToYamlString("name", name, "age", age, "sex", sex);
    }
};

RIGISTER_SOLAR_CONFIG(Person)

TEST(ConfigTest, CustomType) {
    Person p;
    p.name = "guo";
    p.age = 22;
    p.sex = true;
    solar::ConfigVar<Person>::ptr pPersonConfig =
        solar::Config::Lookup("class.person", p, "class person");
    EXPECT_EQ(pPersonConfig->getValue(), p);

    YAML::Node root = YAML::LoadFile("test.yml");
    solar::Config::LoadFromYaml(root);

    Person res;
    res.name = "guovsea";
    res.age = 18;
    res.sex = true;
    EXPECT_EQ(pPersonConfig->getValue(), res);
}

TEST(ConfigTest, CallBack) {
    auto callback = [](const int &oldv, const int &newv) {
        SOLAR_LOG_DEBUG(SOLAR_LOG_ROOT())
            << "old value = " << oldv << " new value = " << newv;
    };
    g_intValueConfig->addListener(1, callback);
    g_intValueConfig->setValue(3333);
}