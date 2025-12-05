#include "../src/editor.h"
#include <gtest/gtest.h>
#include <sstream>
#include <fstream>

TEST(EditorTest, AddAndUnique)
{
    Editor e;
    EXPECT_TRUE(e.addNPC(Type::Thief, "t1", 10, 10));
    EXPECT_FALSE(e.addNPC(Type::Thief, "t1", 20, 20));
    EXPECT_FALSE(e.addNPC(Type::Elf, "e1", 600, 10));
}

TEST(EditorTest, SaveLoadPrint)
{
    Editor e;
    e.addNPC(Type::Thief, "t1", 10, 10);
    e.addNPC(Type::Knight, "k1", 50, 50);
    ASSERT_TRUE(e.save("test_save.txt"));
    Editor e2;
    ASSERT_TRUE(e2.load("test_save.txt"));
    std::ostringstream ss;
    e2.print(ss);
    std::string out = ss.str();
    EXPECT_NE(out.find("t1"), std::string::npos);
    EXPECT_NE(out.find("k1"), std::string::npos);
}

TEST(EditorTest, BattleOutcomeAndObservers)
{
    std::remove("log.txt");
    Editor e;
    e.addNPC(Type::Thief, "th", 10, 10);
    e.addNPC(Type::Elf, "el", 11, 11);
    e.addNPC(Type::Knight, "kn", 100, 100);
    auto consoleObs = std::make_shared<FileObserver>("log.txt");
    auto fileObs = std::make_shared<FileObserver>("log.txt");
    e.attachObserver(consoleObs);
    e.attachObserver(fileObs);
    std::ostringstream capture;
    std::streambuf *old = std::cout.rdbuf(capture.rdbuf());
    e.attachObserver(std::make_shared<ConsoleObserver>());
    e.startBattle(5.0);
    std::cout.rdbuf(old);
    std::string out = capture.str();
    EXPECT_NE(out.find("el was killed by th"), std::string::npos);
    std::ifstream f("log.txt");
    std::string content;
    std::getline(f, content);
    EXPECT_NE(content.find("el was killed by th"), std::string::npos);
}
