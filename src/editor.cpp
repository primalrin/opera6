#include "editor.h"
#include <fstream>
#include <sstream>
#include <cmath>
#include <algorithm>

std::string NPC::toString() const
{
    std::ostringstream ss;
    ss << (type() == Type::Thief ? "Thief" : type() == Type::Knight ? "Knight"
                                                                    : "Elf")
       << " " << name << " " << x << " " << y;
    return ss.str();
}

Type Thief::type() const { return Type::Thief; }
void Thief::accept(BattleVisitor &v, size_t idx) { v.visit(*this, idx); }
Type Knight::type() const { return Type::Knight; }
void Knight::accept(BattleVisitor &v, size_t idx) { v.visit(*this, idx); }
Type Elf::type() const { return Type::Elf; }
void Elf::accept(BattleVisitor &v, size_t idx) { v.visit(*this, idx); }

void DeathNotifier::attach(std::shared_ptr<Observer> o) { observers.push_back(o); }
void DeathNotifier::notify(const std::string &victim, const std::string &killer, bool mutual)
{
    for (auto &o : observers)
        o->onDeath(victim, killer, mutual);
}

bool Editor::addNPC(Type t, const std::string &name, int x, int y)
{
    if (x < 0 || x > 500 || y < 0 || y > 500)
        return false;
    for (auto &p : list)
        if (p->name == name)
            return false;
    list.push_back(NPCFactory::create(t, name, x, y));
    return true;
}
bool Editor::save(const std::string &filename) const
{
    std::ofstream f(filename);
    if (!f)
        return false;
    for (auto &p : list)
        f << (p->type() == Type::Thief ? "Thief" : p->type() == Type::Knight ? "Knight"
                                                                             : "Elf")
          << " " << p->name << " " << p->x << " " << p->y << "\n";
    return true;
}
bool Editor::load(const std::string &filename)
{
    std::ifstream f(filename);
    if (!f)
        return false;
    list.clear();
    std::string ty, name;
    int x, y;
    while (f >> ty >> name >> x >> y)
    {
        auto p = NPCFactory::create(ty, name, x, y);
        if (!p)
            return false;
        for (auto &q : list)
            if (q->name == name)
                return false;
        list.push_back(p);
    }
    return true;
}
void Editor::print(std::ostream &os) const
{
    for (auto &p : list)
        os << (p->type() == Type::Thief ? "Thief" : p->type() == Type::Knight ? "Knight"
                                                                              : "Elf")
           << " " << p->name << " (" << p->x << "," << p->y << ")\n";
}
void Editor::attachObserver(std::shared_ptr<Observer> o) { notifier.attach(o); }
std::vector<std::shared_ptr<NPC>> Editor::npcs() { return list; }

std::shared_ptr<NPC> NPCFactory::create(const std::string &type, const std::string &name, int x, int y)
{
    if (type == "Thief")
        return std::make_shared<Thief>(name, x, y);
    if (type == "Knight")
        return std::make_shared<Knight>(name, x, y);
    if (type == "Elf")
        return std::make_shared<Elf>(name, x, y);
    return nullptr;
}
std::shared_ptr<NPC> NPCFactory::create(Type t, const std::string &name, int x, int y)
{
    if (t == Type::Thief)
        return std::make_shared<Thief>(name, x, y);
    if (t == Type::Knight)
        return std::make_shared<Knight>(name, x, y);
    return std::make_shared<Elf>(name, x, y);
}

bool BattleVisitor::withinRange(size_t i, size_t j) const
{
    auto &a = *allRef[i];
    auto &b = *allRef[j];
    double dx = a.x - b.x;
    double dy = a.y - b.y;
    return std::sqrt(dx * dx + dy * dy) <= range;
}

bool BattleVisitor::kills(Type a, Type b)
{
    if (a == Type::Thief && b == Type::Elf)
        return true;
    if (a == Type::Elf && b == Type::Knight)
        return true;
    if (a == Type::Knight && b == Type::Thief)
        return true;
    return false;
}

void BattleVisitor::visit(Thief &t, size_t idx)
{
    if (!inited)
    {
        dead.assign(allRef.size(), false);
        inited = true;
    }
    if (dead[idx] || !t.alive)
        return;
    for (size_t j = idx + 1; j < allRef.size(); ++j)
    {
        if (dead[j] || !allRef[j]->alive)
            continue;
        if (!withinRange(idx, j))
            continue;
        Type a = Type::Thief;
        Type b = allRef[j]->type();
        if (a == b)
        {
            dead[idx] = dead[j] = true;
            allRef[idx]->alive = false;
            allRef[j]->alive = false;
            notifier.notify(allRef[j]->name, allRef[idx]->name, true);
            notifier.notify(allRef[idx]->name, allRef[j]->name, true);
            break;
        }
        if (kills(a, b))
        {
            dead[j] = true;
            allRef[j]->alive = false;
            notifier.notify(allRef[j]->name, allRef[idx]->name, false);
        }
        else if (kills(b, a))
        {
            dead[idx] = true;
            allRef[idx]->alive = false;
            notifier.notify(allRef[idx]->name, allRef[j]->name, false);
            break;
        }
    }
}

void BattleVisitor::visit(Knight &k, size_t idx)
{
    if (!inited)
    {
        dead.assign(allRef.size(), false);
        inited = true;
    }
    if (dead[idx] || !k.alive)
        return;
    for (size_t j = idx + 1; j < allRef.size(); ++j)
    {
        if (dead[j] || !allRef[j]->alive)
            continue;
        if (!withinRange(idx, j))
            continue;
        Type a = Type::Knight;
        Type b = allRef[j]->type();
        if (a == b)
        {
            dead[idx] = dead[j] = true;
            allRef[idx]->alive = false;
            allRef[j]->alive = false;
            notifier.notify(allRef[j]->name, allRef[idx]->name, true);
            notifier.notify(allRef[idx]->name, allRef[j]->name, true);
            break;
        }
        if (kills(a, b))
        {
            dead[j] = true;
            allRef[j]->alive = false;
            notifier.notify(allRef[j]->name, allRef[idx]->name, false);
        }
        else if (kills(b, a))
        {
            dead[idx] = true;
            allRef[idx]->alive = false;
            notifier.notify(allRef[idx]->name, allRef[j]->name, false);
            break;
        }
    }
}

void BattleVisitor::visit(Elf &e, size_t idx)
{
    if (!inited)
    {
        dead.assign(allRef.size(), false);
        inited = true;
    }
    if (dead[idx] || !e.alive)
        return;
    for (size_t j = idx + 1; j < allRef.size(); ++j)
    {
        if (dead[j] || !allRef[j]->alive)
            continue;
        if (!withinRange(idx, j))
            continue;
        Type a = Type::Elf;
        Type b = allRef[j]->type();
        if (a == b)
        {
            dead[idx] = dead[j] = true;
            allRef[idx]->alive = false;
            allRef[j]->alive = false;
            notifier.notify(allRef[j]->name, allRef[idx]->name, true);
            notifier.notify(allRef[idx]->name, allRef[j]->name, true);
            break;
        }
        if (kills(a, b))
        {
            dead[j] = true;
            allRef[j]->alive = false;
            notifier.notify(allRef[j]->name, allRef[idx]->name, false);
        }
        else if (kills(b, a))
        {
            dead[idx] = true;
            allRef[idx]->alive = false;
            notifier.notify(allRef[idx]->name, allRef[j]->name, false);
            break;
        }
    }
}

void BattleVisitor::runAll()
{
    for (size_t i = 0; i < allRef.size(); ++i)
    {
        if (!allRef[i]->alive)
            continue;
        allRef[i]->accept(*this, i);
    }
    size_t write = 0;
    for (size_t i = 0; i < allRef.size(); ++i)
        if (!dead[i])
        {
            allRef[write++] = allRef[i];
        }
    allRef.resize(write);
}

void ConsoleObserver::onDeath(const std::string &victim, const std::string &killer, bool mutual)
{
    if (mutual)
        std::cout << victim << " and " << killer << " killed each other\n";
    else
        std::cout << victim << " was killed by " << killer << "\n";
}

FileObserver::FileObserver(const std::string &fname) : filename(fname) {}
void FileObserver::onDeath(const std::string &victim, const std::string &killer, bool mutual)
{
    std::ofstream f(filename, std::ios::app);
    if (mutual)
        f << victim << " and " << killer << " killed each other\n";
    else
        f << victim << " was killed by " << killer << "\n";
}

void Editor::startBattle(double range)
{
    BattleVisitor bv(list, notifier, range);
    bv.runAll();
}
