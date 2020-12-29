#pragma once

#include "common.h"
#include "value.h"
#include "chunk.h"
#include "table.h"

#define OBJ_TYPE(value)        (AS_OBJ(value)->type)
#define IS_STRING(value)       isObjType(value, OBJ_STRING)
#define AS_STRING(value)       ((ObjString*)AS_OBJ(value))
#define AS_CSTRING(value)      (((ObjString*)AS_OBJ(value))->chars)
#define IS_FUNCTION(value)     isObjType(value, OBJ_FUNCTION)
#define AS_FUNCTION(value)     ((ObjFunction*)AS_OBJ(value))
#define IS_NATIVE(value)       isObjType(value, OBJ_NATIVE)
#define AS_NATIVE(value)       (((ObjNative*)AS_OBJ(value))->function)
#define IS_CLOSURE(value)      isObjType(value, OBJ_CLOSURE)
#define AS_CLOSURE(value)      ((ObjClosure*)AS_OBJ(value))
#define IS_CLASS(value)        isObjType(value, OBJ_CLASS)
#define AS_CLASS(value)        ((ObjClass*)AS_OBJ(value))
#define IS_INSTANCE(value)     isObjType(value, OBJ_INSTANCE)
#define AS_INSTANCE(value)     ((ObjInstance*)AS_OBJ(value))
#define IS_BOUND_METHOD(value) isObjType(value, OBJ_BOUND_METHOD)
#define AS_BOUND_METHOD(value) ((ObjBoundMethod*)AS_OBJ(value))

typedef enum {
    OBJ_STRING,
    OBJ_FUNCTION,
    OBJ_CLOSURE,
    OBJ_UPVALUE,
    OBJ_CLASS,
    OBJ_INSTANCE,
    OBJ_BOUND_METHOD,
    OBJ_NATIVE
} ObjType;

struct Obj {
    ObjType type;
    bool isMarked;
    struct Obj* next;
};

struct ObjString {
    Obj obj;
    int length;
    char* chars;
    uint32_t hash;
};

ObjString* copyString(const char* chars, int length);

void printObject(Value value);

ObjString* takeString(char* chars, int length);

static inline bool isObjType(Value value, ObjType type)
{
    return IS_OBJ(value) && AS_OBJ(value)->type == type;
}

typedef struct
{
    Obj obj;
    int upvalueCount;
    int arity;
    Chunk chunk;
    ObjString* name;
} ObjFunction;

ObjFunction* newFunction();

typedef Value (*NativeFn)(int argCount, Value* args);

typedef struct
{
    Obj obj;
    NativeFn function;
} ObjNative;

ObjNative* newNative(NativeFn function);

typedef struct ObjUpvalue
{
    Obj obj;
    Value* location;
    struct ObjUpvalue* next;
    Value closed;
} ObjUpvalue;

ObjUpvalue* newUpvalue(Value* slot);

typedef struct
{
    Obj obj;
    ObjFunction* function;
    ObjUpvalue** upvalues;
    int upvalueCount;
} ObjClosure;

ObjClosure* newClosure(ObjFunction* function);

typedef struct
{
    Obj obj;
    ObjString* name;
    Table methods;
} ObjClass;

ObjClass* newClass(ObjString* name);

typedef struct
{
    Obj obj;
    ObjClass* klass;
    Table fields;
} ObjInstance;


ObjInstance* newInstance(ObjClass* klass);

typedef struct
{
    Obj obj;
    Value receiver;
    ObjClosure* method;
} ObjBoundMethod;

ObjBoundMethod* newBoundMethod(Value receiver, ObjClosure* method);


