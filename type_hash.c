#include <config.h>

#include "type_hash.h"

#include "adt/error.h"
#include "ast_t.h"

#include <assert.h>

#define HashSet         type_hash_t
#define HashSetIterator type_hash_iterator_t
#define ValueType       type_t*
#include "adt/hashset.h"
#undef ValueType
#undef HashSetIterator
#undef HashSet

static
unsigned hash_ptr(const void *ptr)
{
	unsigned ptr_int = (ptr - NULL);
	return ptr_int >> 3;
}

static
unsigned hash_atomic_type(const atomic_type_t *type)
{
	unsigned some_prime = 27644437;

	return type->atype * some_prime;
}

static
unsigned hash_struct_type(const struct_type_t *type)
{
	(void) type;
	assert(0);
}

static
unsigned hash_type(const type_t *type);

static
unsigned hash_method_type(const method_type_t *type)
{
	unsigned result = hash_ptr(type->result_type);

	method_parameter_type_t *parameter = type->parameter_types;
	while(parameter != NULL) {
		result ^= hash_ptr(parameter->type);
		parameter = parameter->next;
	}
	result ^= hash_ptr(type->abi_style);

	return result;
}

static
unsigned hash_type(const type_t *type)
{
	switch(type->type) {
	case TYPE_INVALID:
	case TYPE_VOID:
	case TYPE_REF:
		panic("internalizing void, invalid or ref types not possible");
	case TYPE_ATOMIC:
		return hash_atomic_type((const atomic_type_t*) type);
	case TYPE_STRUCT:
		return hash_struct_type((const struct_type_t*) type);
	case TYPE_METHOD:
		return hash_method_type((const method_type_t*) type);
	default:
		assert(0);
	}
}

static
int atomic_types_equal(const atomic_type_t *type1, const atomic_type_t *type2)
{
	return type1->atype == type2->atype;
}

static
int struct_types_equal(const struct_type_t *type1, const struct_type_t *type2)
{
	(void) type1;
	(void) type2;
	assert(0);
	return 0;
}

static
int method_types_equal(const method_type_t *type1, const method_type_t *type2)
{
	if(type1->result_type != type2->result_type)
		return 0;

	if(type1->abi_style != type2->abi_style)
		return 0;

	method_parameter_type_t *param1 = type1->parameter_types;
	method_parameter_type_t *param2 = type2->parameter_types;
	while(param1 != NULL && param2 != NULL) {
		if(param1->type != param2->type)
			return 0;
		param1 = param1->next;
		param2 = param2->next;
	}
	if(param1 != NULL || param2 != NULL)
		return 0;

	return 1;
}

static
int types_equal(const type_t *type1, const type_t *type2)
{
	if(type1 == type2)
		return 1;
	if(type1->type != type2->type)
		return 0;

	switch(type1->type) {
	case TYPE_INVALID:
	case TYPE_VOID:
	case TYPE_REF:
		return 0;
	case TYPE_ATOMIC:
		return atomic_types_equal((const atomic_type_t*) type1,
		                          (const atomic_type_t*) type2);
	case TYPE_STRUCT:
		return struct_types_equal((const struct_type_t*) type1,
		                          (const struct_type_t*) type2);
	case TYPE_METHOD:
		return method_types_equal((const method_type_t*) type1,
		                          (const method_type_t*) type2);
	default:
		assert(0);
	}
}

#define HashSet                    type_hash_t
#define HashSetIterator            type_hash_iterator_t
#define ValueType                  type_t*
#define NullValue                  NULL
#define DeletedValue               ((type_t*)-1)
#define Hash(this, key)            hash_type(key)
#define KeysEqual(this,key1,key2)  types_equal(key1, key2)
#define SetRangeEmpty(ptr,size)    memset(ptr, 0, (size) * sizeof(*(ptr)))

#define hashset_init             _typehash_init
#define hashset_init_size        _typehash_init_size
#define hashset_destroy          _typehash_destroy
#define hashset_insert           _typehash_insert
#define hashset_remove           typehash_remove
#define hashset_find             typehash_find
#define hashset_size             typehash_size
#define hashset_iterator_init    typehash_iterator_init
#define hashset_iterator_next    typehash_iterator_next
#define hashset_remove_iterator  typehash_remove_iterator

#include "adt/hashset.c"

static type_hash_t typehash;

void typehash_init(void)
{
	_typehash_init(&typehash);
}

void typehash_destroy(void)
{
	_typehash_destroy(&typehash);
}

type_t *typehash_insert(type_t *type)
{
	return _typehash_insert(&typehash, type);
}
