#ifndef REFERENCES_H
#define REFERENCES_H

#include "cpu.h"
#include "memory.h"

template <class T>
class Reference
{
public:
    virtual ~Reference() {};
    virtual T get() = 0;
    virtual void set(T v) = 0;
};

template <class T>
class RegisterReference : public Reference<T>
{
private:
    T &r;
public:
    RegisterReference(T &r) : r(r) {};
    virtual T get() { return r; };
    virtual void set(T v) { r = v; };
};

template <class T>
class MemoryReference : public Reference<T>
{
private:
    CPU *cpu;
    word &r;
public:
    MemoryReference(CPU *cpu, word &r) : cpu(cpu), r(r) {};
    virtual T get() {
        return cpu->memory->get<T>(r);
    };
    virtual void set(T v) {
        cpu->memory->set<T>(r, v);
    };
};

template <class T>
class Memory_HL_Reference : public Reference<T>
{
private:
    CPU *cpu;
    word &r;
    word add;
public:
    Memory_HL_Reference(CPU *cpu, word &r, word add) : cpu(cpu), r(r), add(add) {};
    virtual T get() {
        T v = cpu->memory->get<T>(r);
        r += add;
        return v;
    };
    virtual void set(T v) {
        cpu->memory->set<T>(r, v);
        r += add;
    };
};

template <class T>
class Memory_a8_Reference : public Reference<T>
{
private:
    CPU *cpu;
    word &r;
public:
    Memory_a8_Reference(CPU *cpu, word &r): cpu(cpu), r(r) {};
    virtual T get() {
        return cpu->memory->get<T>( word(cpu->memory->get<byte>(r), 0xff));
    };
    virtual void set(T v) {
        cpu->memory->set<T>(word(cpu->memory->get<byte>(r), 0xff), v);
    };
};

template <class T>
class Memory_a16_Reference : public Reference<T>
{
private:
    CPU *cpu;
    word &r;
public:
    Memory_a16_Reference(CPU *cpu, word &r): cpu(cpu), r(r) {};
    virtual T get() {
        return cpu->memory->get<T>( cpu->memory->get<word>(r) );
    };
    virtual void set(T v) {
        cpu->memory->set<T>( cpu->memory->get<word>(r), v );
    };
};


template <class T>
class Memory_SingleRegister_Reference : public Reference<T>
{
private:
    CPU *cpu;
    byte &r;
public:
    Memory_SingleRegister_Reference(CPU *cpu, byte &r): cpu(cpu), r(r) {};
    virtual T get() {
        return cpu->memory->get<T>( word(r, 0xff));
    };
    virtual void set(T v) {
        cpu->memory->set<T>(word(r, 0xff), v);
    };
};

#endif
