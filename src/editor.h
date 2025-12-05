#pragma once
#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <iostream>

enum class Type
{
    Thief,
    Knight,
    Elf
};

class BattleVisitor;

class NPC
{
public:
    std::string name;
    int x;
    int y;
    bool alive = true;
    NPC(std::string n, int xx, int yy) : name(std::move(n)), x(xx), y(yy) {}
    virtual ~NPC() = default;
    virtual Type type() const = 0;
    virtual void accept(BattleVisitor &v, size_t idx) = 0;
    std::string toString() const;
};

class Thief : public NPC
{
public:
    Thief(std::string n, int xx, int yy) : NPC(n, xx, yy) {}
    Type type() const override;
    void accept(BattleVisitor &v, size_t idx) override;
};
class Knight : public NPC
{
public:
    Knight(std::string n, int xx, int yy) : NPC(n, xx, yy) {}
    Type type() const override;
    void accept(BattleVisitor &v, size_t idx) override;
};
class Elf : public NPC
{
public:
    Elf(std::string n, int xx, int yy) : NPC(n, xx, yy) {}
    Type type() const override;
    void accept(BattleVisitor &v, size_t idx) override;
};

class Observer
{
public:
    virtual ~Observer() = default;
    virtual void onDeath(const std::string &victim, const std::string &killer, bool mutual) = 0;
};

class DeathNotifier
{
public:
    void attach(std::shared_ptr<Observer> o);
    void notify(const std::string &victim, const std::string &killer, bool mutual);

private:
    std::vector<std::shared_ptr<Observer>> observers;
};

class Editor
{
public:
    bool addNPC(Type t, const std::string &name, int x, int y);
    bool save(const std::string &filename) const;
    bool load(const std::string &filename);
    void print(std::ostream &os) const;
    void startBattle(double range);
    void attachObserver(std::shared_ptr<Observer> o);
    std::vector<std::shared_ptr<NPC>> npcs();

private:
    std::vector<std::shared_ptr<NPC>> list;
    DeathNotifier notifier;
    friend class NPCFactory;
};

class NPCFactory
{
public:
    static std::shared_ptr<NPC> create(const std::string &type, const std::string &name, int x, int y);
    static std::shared_ptr<NPC> create(Type t, const std::string &name, int x, int y);
};

class BattleVisitor
{
public:
    BattleVisitor(std::vector<std::shared_ptr<NPC>> &all, DeathNotifier &n, double r) : allRef(all), notifier(n), range(r) {}
    void visit(Thief &t, size_t idx);
    void visit(Knight &k, size_t idx);
    void visit(Elf &e, size_t idx);
    void runAll();

private:
    std::vector<std::shared_ptr<NPC>> &allRef;
    DeathNotifier &notifier;
    double range;
    std::vector<bool> dead;
    bool inited = false;
    bool withinRange(size_t i, size_t j) const;
    static bool kills(Type a, Type b);
};

class ConsoleObserver : public Observer
{
public:
    ConsoleObserver() = default;
    void onDeath(const std::string &victim, const std::string &killer, bool mutual) override;
};

class FileObserver : public Observer
{
public:
    FileObserver(const std::string &fname);
    void onDeath(const std::string &victim, const std::string &killer, bool mutual) override;

private:
    std::string filename;
};
