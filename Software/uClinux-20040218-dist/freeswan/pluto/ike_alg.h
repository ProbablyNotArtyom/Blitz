#ifndef _IKE_ALG_H
#define _IKE_ALG_H

#define IKE_ALG_COMMON \
	u_int16_t algo_type;		\
	u_int16_t algo_id;		\
	struct ike_alg *algo_next
struct ike_alg {
    IKE_ALG_COMMON;
};
struct encrypt_desc {
    IKE_ALG_COMMON;
    size_t enc_ctxsize;
    size_t enc_blocksize;
    unsigned keydeflen;
    unsigned keymaxlen;
    unsigned keyminlen;
    void (*do_crypt)(u_int8_t *dat, size_t datasize, u_int8_t *key, size_t key_size, u_int8_t *iv, bool enc);
};
struct hash_desc {
    IKE_ALG_COMMON;
    size_t hash_ctx_size;
    size_t hash_digest_size;
    void (*hash_init)(void *ctx);
    void (*hash_update)(void *ctx, const u_int8_t *in, size_t datasize);
    void (*hash_final)(u_int8_t *out, void *ctx);
};
struct db_context * ike_alg_db_new(struct alg_info_ike *ai, lset_t policy);
void ike_alg_show_status(void);
void ike_alg_show_connection(struct connection *c, const char *instance);

#define IKE_EALG_FOR_EACH(a) \
	for(a=ike_alg_base[IKE_ALG_ENCRYPT];a;a=a->algo_next)
#define IKE_HALG_FOR_EACH(a) \
	for(a=ike_alg_base[IKE_ALG_HASH];a;a=a->algo_next)
bool ike_alg_enc_present(int ealg);
bool ike_alg_hash_present(int halg);
bool ike_alg_enc_ok(int ealg, unsigned key_len, struct alg_info_ike *alg_info_ike, const char **);

int ike_alg_init(void);

/*	
 *	This could be just OAKLEY_XXXXXX_ALGORITHM, but it's
 *	here with other name as a way to assure that the
 *	algorithm hook type is supported (detected at compile time)
 */
#define IKE_ALG_ENCRYPT	0
#define IKE_ALG_HASH	1
#define IKE_ALG_MAX	1
extern struct ike_alg *ike_alg_base[IKE_ALG_MAX+1];
int ike_alg_add(struct ike_alg *);
int ike_alg_register_enc(struct encrypt_desc *e);
int ike_alg_register_hash(struct hash_desc *a);
struct ike_alg *ike_alg_find(unsigned algo_type, unsigned algo_id, unsigned keysize);
static __inline__ struct hash_desc *ike_alg_get_hasher(int alg)
{
	return (struct hash_desc *) ike_alg_find(IKE_ALG_HASH, alg, 0);
}
static __inline__ struct encrypt_desc *ike_alg_get_encrypter(int alg)
{
	return (struct encrypt_desc *) ike_alg_find(IKE_ALG_ENCRYPT, alg, 0);
}
#endif /* _IKE_ALG_H */
