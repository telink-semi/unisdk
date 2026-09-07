#define MBEDTLS_DECLARE_PRIVATE_IDENTIFIERS

#include <mbedtls/build_info.h>
#include <mbedtls/error.h>
#include <mbedtls/private/ecp.h>

#ifdef MBEDTLS_ECP_C

/* ECP HW backend finctionality */
extern int tlk_mbedtls_ecp_check_pubkey(const mbedtls_ecp_group* grp, const mbedtls_ecp_point* pt);
extern int tlk_mbedtls_ecp_mul_restartable(mbedtls_ecp_group* grp, mbedtls_ecp_point* R,
                                           const mbedtls_mpi* m, const mbedtls_ecp_point*     P,
                                           int (*f_rng)(void*, unsigned char*, size_t), void* p_rng,
                                           mbedtls_ecp_restart_ctx* rs_ctx);
extern int tlk_mbedtls_ecp_muladd_restartable(mbedtls_ecp_group* grp, mbedtls_ecp_point* R,
                                              const mbedtls_mpi* m, const mbedtls_ecp_point* P,
                                              const mbedtls_mpi* n, const mbedtls_ecp_point* Q,
                                              mbedtls_ecp_restart_ctx* rs_ctx);

/* ECP MbedTLS implementation */
extern int __real_mbedtls_ecp_check_pubkey(const mbedtls_ecp_group* grp,
                                           const mbedtls_ecp_point* pt);
extern int __real_mbedtls_ecp_mul_restartable(mbedtls_ecp_group* grp, mbedtls_ecp_point* R,
                                              const mbedtls_mpi* m, const mbedtls_ecp_point* P,
                                              int (*f_rng)(void*, unsigned char*, size_t),
                                              void* p_rng, mbedtls_ecp_restart_ctx* rs_ctx);
extern int __real_mbedtls_ecp_muladd_restartable(mbedtls_ecp_group* grp, mbedtls_ecp_point* R,
                                                 const mbedtls_mpi* m, const mbedtls_ecp_point* P,
                                                 const mbedtls_mpi* n, const mbedtls_ecp_point* Q,
                                                 mbedtls_ecp_restart_ctx* rs_ctx);

int __wrap_mbedtls_ecp_check_pubkey(const mbedtls_ecp_group* grp, const mbedtls_ecp_point* pt)
{
    int result = tlk_mbedtls_ecp_check_pubkey(grp, pt);

    if (result == MBEDTLS_ERR_PLATFORM_HW_ACCEL_FAILED)
    {
        result = __real_mbedtls_ecp_check_pubkey(grp, pt);
    }
    return result;
}

int __wrap_mbedtls_ecp_mul_restartable(mbedtls_ecp_group* grp, mbedtls_ecp_point* R,
                                       const mbedtls_mpi* m, const mbedtls_ecp_point*     P,
                                       int (*f_rng)(void*, unsigned char*, size_t), void* p_rng,
                                       mbedtls_ecp_restart_ctx* rs_ctx)
{
    int result = tlk_mbedtls_ecp_mul_restartable(grp, R, m, P, f_rng, p_rng, rs_ctx);

    if (result == MBEDTLS_ERR_PLATFORM_HW_ACCEL_FAILED)
    {
        result = __real_mbedtls_ecp_mul_restartable(grp, R, m, P, f_rng, p_rng, rs_ctx);
    }
    return result;
}

int __wrap_mbedtls_ecp_mul(mbedtls_ecp_group* grp, mbedtls_ecp_point* R, const mbedtls_mpi* m,
                           const mbedtls_ecp_point* P, int (*f_rng)(void*, unsigned char*, size_t),
                           void*                    p_rng)
{

    return __wrap_mbedtls_ecp_mul_restartable(grp, R, m, P, f_rng, p_rng, NULL);
}

int __wrap_mbedtls_ecp_muladd_restartable(mbedtls_ecp_group* grp, mbedtls_ecp_point* R,
                                          const mbedtls_mpi* m, const mbedtls_ecp_point* P,
                                          const mbedtls_mpi* n, const mbedtls_ecp_point* Q,
                                          mbedtls_ecp_restart_ctx* rs_ctx)
{
    int result = tlk_mbedtls_ecp_muladd_restartable(grp, R, m, P, n, Q, rs_ctx);

    if (result == MBEDTLS_ERR_PLATFORM_HW_ACCEL_FAILED)
    {
        result = __real_mbedtls_ecp_muladd_restartable(grp, R, m, P, n, Q, rs_ctx);
    }
    return result;
}

int __wrap_mbedtls_ecp_muladd(mbedtls_ecp_group* grp, mbedtls_ecp_point* R, const mbedtls_mpi* m,
                              const mbedtls_ecp_point* P, const mbedtls_mpi* n,
                              const mbedtls_ecp_point* Q)
{
    return __wrap_mbedtls_ecp_muladd_restartable(grp, R, m, P, n, Q, NULL);
}

int __wrap_mbedtls_ecp_gen_keypair_base(mbedtls_ecp_group* grp, const mbedtls_ecp_point* G,
                                        mbedtls_mpi* d, mbedtls_ecp_point*                 Q,
                                        int (*f_rng)(void*, unsigned char*, size_t), void* p_rng)
{
    int result = mbedtls_ecp_gen_privkey(grp, d, f_rng, p_rng);

    if (!result)
    {
        result = __wrap_mbedtls_ecp_mul(grp, Q, d, G, f_rng, p_rng);
    }
    return result;
}

int __wrap_mbedtls_ecp_gen_keypair(mbedtls_ecp_group* grp, mbedtls_mpi* d, mbedtls_ecp_point* Q,
                                   int (*f_rng)(void*, unsigned char*, size_t), void* p_rng)
{
    return __wrap_mbedtls_ecp_gen_keypair_base(grp, &grp->G, d, Q, f_rng, p_rng);
}

int __wrap_mbedtls_ecp_gen_key(mbedtls_ecp_group_id grp_id, mbedtls_ecp_keypair*  key,
                               int (*f_rng)(void*, unsigned char*, size_t), void* p_rng)
{
    int result = mbedtls_ecp_group_load(&key->MBEDTLS_PRIVATE(grp), grp_id);
    if (!result)
    {
        result = __wrap_mbedtls_ecp_gen_keypair(&key->MBEDTLS_PRIVATE(grp),
                                                &key->MBEDTLS_PRIVATE(d),
                                                &key->MBEDTLS_PRIVATE(Q),
                                                f_rng,
                                                p_rng);
    }
    return result;
}

#endif /* MBEDTLS_ECP_C */
