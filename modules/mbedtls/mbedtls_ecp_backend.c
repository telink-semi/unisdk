#define MBEDTLS_DECLARE_PRIVATE_IDENTIFIERS

#include <mbedtls/build_info.h>
#include <mbedtls/error.h>
#include <mbedtls/private/ecp.h>

#ifdef MBEDTLS_ECP_C

#if TLK_IS_ENABLED(CONFIG_TLK_CRYPTO_PKE)

#include <common/include/tlk_init.h>
#if defined(CONFIG_TLK_SOC_SERIES_TL321X) || defined(CONFIG_TLK_SOC_SERIES_TL721X)
#include <crypto/include/crypto_common/utility.h>
#include <crypto/include/pke/pke_portable.h>
#endif /* CONFIG_TLK_SOC_SERIES_TL321X || CONFIG_TLK_SOC_SERIES_TL721X */
#include <crypto/include/pke/pke.h>
#include <system/debug/log/tlk_log.h>

TLK_LOG_CREATE(mbedtls_pke, "MBEDTLS PKE");

extern void tlk_mbedtls_ecp_lock(void);
extern void tlk_mbedtls_ecp_unlock(void);

/****************************************************************
 * HW unit curve data constants definition
 ****************************************************************/

#ifndef PKE_OPERAND_MAX_WORD_LEN
#define PKE_OPERAND_MAX_WORD_LEN 8
#endif /* PKE_OPERAND_MAX_WORD_LEN */

#if defined(CONFIG_TLK_SOC_SERIES_TLSR952X)
#define ECCP_P_BITLEN eccp_p_bitLen
#define ECCP_N_BITLEN eccp_n_bitLen
#define ECCP_P        eccp_p
#define ECCP_P_H      eccp_p_h
#define ECCP_P_N      eccp_p_n1
#define ECCP_A        eccp_a
#define ECCP_B        eccp_b
#define MONT_P_BITLEN p_bitLen
#define MONT_P        p
#define MONT_P_H      p_h
#define MONT_P_N0     p_n1
#define MONT_A24      a24
#elif defined(CONFIG_TLK_SOC_SERIES_TL321X)
#define ECCP_P_BITLEN eccp_p_bitLen
#define ECCP_N_BITLEN eccp_n_bitLen
#define ECCP_P        eccp_p
#define ECCP_P_H      eccp_p_h
#define ECCP_P_N      eccp_p_n0
#define ECCP_A        eccp_a
#define ECCP_B        eccp_b
#define MONT_P_BITLEN p_bitLen
#define MONT_P        p
#define MONT_P_H      p_h
#define MONT_P_N0     p_n0
#define MONT_A24      a24
#elif defined(CONFIG_TLK_SOC_SERIES_TL721X)
#define ECCP_P_BITLEN eccp_p_bitLen
#define ECCP_N_BITLEN eccp_n_bitLen
#define ECCP_P        eccp_p
#define ECCP_P_H      eccp_p_h
#define ECCP_P_N      eccp_p_n1
#define ECCP_A        eccp_a
#define ECCP_B        eccp_b
#define MONT_P_BITLEN p_bitLen
#define MONT_P        p
#define MONT_P_H      p_h
#define MONT_P_N0     p_n0
#define MONT_A24      a24
#else
#error "Unsupported TLK SOC series!"
#endif

#ifdef MBEDTLS_ECP_DP_SECP256R1_ENABLED
static eccp_curve_t tlk_mbedtls_secp256r1_curve_data = {
    .ECCP_P_BITLEN = 256,
    .ECCP_N_BITLEN = 256,
    .ECCP_P =
        (unsigned int[]){
            0xffffffff,
            0xffffffff,
            0xffffffff,
            0x00000000,
            0x00000000,
            0x00000000,
            0x00000001,
            0xffffffff,
        },
    .ECCP_P_H =
        (unsigned int[]){
            0x00000003,
            0x00000000,
            0xffffffff,
            0xfffffffb,
            0xfffffffe,
            0xffffffff,
            0xfffffffd,
            0x00000004,
        },
    .ECCP_P_N =
        (unsigned int[]){
            0x00000001,
        },
    .ECCP_A =
        (unsigned int[]){
            0xfffffffc,
            0xffffffff,
            0xffffffff,
            0x00000000,
            0x00000000,
            0x00000000,
            0x00000001,
            0xffffffff,
        },
    .ECCP_B =
        (unsigned int[]){
            0x27d2604b,
            0x3bce3c3e,
            0xcc53b0f6,
            0x651d06b0,
            0x769886bc,
            0xb3ebbd55,
            0xaa3a93e7,
            0x5ac635d8,
        },
};
#endif /* MBEDTLS_ECP_DP_SECP256R1_ENABLED */

#ifdef MBEDTLS_ECP_DP_SECP256K1_ENABLED
static eccp_curve_t tlk_mbedtls_secp256k1_curve_data = {
    .ECCP_P_BITLEN = 256,
    .ECCP_N_BITLEN = 256,
    .ECCP_P =
        (unsigned int[]){
            0xfffffc2f,
            0xfffffffe,
            0xffffffff,
            0xffffffff,
            0xffffffff,
            0xffffffff,
            0xffffffff,
            0xffffffff,
        },
    .ECCP_P_H =
        (unsigned int[]){
            0x000e90a1,
            0x000007a2,
            0x00000001,
            0x00000000,
            0x00000000,
            0x00000000,
            0x00000000,
            0x00000000,
        },
    .ECCP_P_N = (unsigned int[]){0xd2253531},
    .ECCP_A =
        (unsigned int[]){
            0x00000000,
            0x00000000,
            0x00000000,
            0x00000000,
            0x00000000,
            0x00000000,
            0x00000000,
            0x00000000,
        },
    .ECCP_B =
        (unsigned int[]){
            0x00000007,
            0x00000000,
            0x00000000,
            0x00000000,
            0x00000000,
            0x00000000,
            0x00000000,
            0x00000000,
        },
};
#endif /* MBEDTLS_ECP_DP_SECP256K1_ENABLED */

#ifdef MBEDTLS_ECP_DP_BP256R1_ENABLED
static eccp_curve_t tlk_mbedtls_bp256r1_curve_data = {
    .ECCP_P_BITLEN = 256,
    .ECCP_N_BITLEN = 256,
    .ECCP_P =
        (unsigned int[]){
            0x1f6e5377,
            0x2013481d,
            0xd5262028,
            0x6e3bf623,
            0x9d838d72,
            0x3e660a90,
            0xa1eea9bc,
            0xa9fb57db,
        },
    .ECCP_P_H =
        (unsigned int[]){
            0xa6465b6c,
            0x8cfedf7b,
            0x614d4f4d,
            0x5cce4c26,
            0x6b1ac807,
            0xa1ecdacd,
            0xe5957fa8,
            0x4717aa21,
        },
    .ECCP_P_N =
        (unsigned int[]){
            0xcefd89b9,
        },
    .ECCP_A =
        (unsigned int[]){
            0xf330b5d9,
            0xe94a4b44,
            0x26dc5c6c,
            0xfb8055c1,
            0x417affe7,
            0xeef67530,
            0xfc2c3057,
            0x7d5a0975,
        },
    .ECCP_B =
        (unsigned int[]){
            0xff8c07b6,
            0x6bccdc18,
            0x5cf7e1ce,
            0x95841629,
            0xbbd77cbf,
            0xf330b5d9,
            0xe94a4b44,
            0x26dc5c6c,
        },
};
#endif /* MBEDTLS_ECP_DP_BP256R1_ENABLED */

#ifdef MBEDTLS_ECP_DP_SECP224R1_ENABLED
static eccp_curve_t tlk_mbedtls_secp224r1_curve_data = {
    .ECCP_P_BITLEN = 224,
    .ECCP_N_BITLEN = 224,
    .ECCP_P =
        (unsigned int[]){
            0x00000001,
            0x00000000,
            0x00000000,
            0xffffffff,
            0xffffffff,
            0xffffffff,
            0xffffffff,
        },
    .ECCP_P_H =
        (unsigned int[]){
            0x00000001,
            0x00000000,
            0x00000000,
            0xfffffffe,
            0xffffffff,
            0xffffffff,
            0x00000000,
        },
    .ECCP_P_N = (unsigned int[]){0xffffffff},
    .ECCP_A =
        (unsigned int[]){
            0xfffffffe,
            0xffffffff,
            0xffffffff,
            0xfffffffe,
            0xffffffff,
            0xffffffff,
            0xffffffff,
        },
    .ECCP_B =
        (unsigned int[]){
            0x2355ffb4,
            0x270b3943,
            0xd7bfd8ba,
            0x5044b0b7,
            0xf5413256,
            0x0c04b3ab,
            0xb4050a85,
        },
};
#endif /* MBEDTLS_ECP_DP_SECP224R1_ENABLED */

#ifdef MBEDTLS_ECP_DP_SECP224K1_ENABLED
static eccp_curve_t tlk_mbedtls_secp224k1_curve_data = {
    .ECCP_P_BITLEN = 224,
    .ECCP_N_BITLEN = 224,
    .ECCP_P =
        (unsigned int[]){
            0xffffe56d,
            0xfffffffe,
            0xffffffff,
            0xffffffff,
            0xffffffff,
            0xffffffff,
            0xffffffff,
        },
    .ECCP_P_H =
        (unsigned int[]){
            0x02c23069,
            0x00003526,
            0x00000001,
            0x00000000,
            0x00000000,
            0x00000000,
            0x00000000,
        },
    .ECCP_P_N = (unsigned int[]){0x198d139b},
    .ECCP_A =
        (unsigned int[]){
            0x00000000,
            0x00000000,
            0x00000000,
            0x00000000,
            0x00000000,
            0x00000000,
            0x00000000,
        },
    .ECCP_B =
        (unsigned int[]){
            0x00000005,
            0x00000000,
            0x00000000,
            0x00000000,
            0x00000000,
            0x00000000,
            0x00000000,
        },
};
#endif /* MBEDTLS_ECP_DP_SECP224K1_ENABLED */

#ifdef MBEDTLS_ECP_DP_SECP192R1_ENABLED
static eccp_curve_t tlk_mbedtls_secp192r1_curve_data = {
    .ECCP_P_BITLEN = 192,
    .ECCP_N_BITLEN = 192,
    .ECCP_P =
        (unsigned int[]){
            0xffffffff,
            0xffffffff,
            0xfffffffe,
            0xffffffff,
            0xffffffff,
            0xffffffff,
        },
    .ECCP_P_H =
        (unsigned int[]){
            0x00000001,
            0x00000000,
            0x00000002,
            0x00000000,
            0x00000001,
            0x00000000,
        },
    .ECCP_P_N = (unsigned int[]){0x00000001},
    .ECCP_A =
        (unsigned int[]){
            0xfffffffc,
            0xffffffff,
            0xfffffffe,
            0xffffffff,
            0xffffffff,
            0xffffffff,
        },
    .ECCP_B =
        (unsigned int[]){
            0xc146b9b1,
            0xfeb8deec,
            0x72243049,
            0x0fa7e9ab,
            0xe59c80e7,
            0x64210519,
        },
};
#endif /* MBEDTLS_ECP_DP_SECP192R1_ENABLED */

#ifdef MBEDTLS_ECP_DP_SECP192K1_ENABLED
static eccp_curve_t tlk_mbedtls_secp192k1_curve_data = {
    .ECCP_P_BITLEN = 192,
    .ECCP_N_BITLEN = 192,
    .ECCP_P =
        (unsigned int[]){
            0xffffee37,
            0xfffffffe,
            0xffffffff,
            0xffffffff,
            0xffffffff,
            0xffffffff,
        },
    .ECCP_P_H =
        (unsigned int[]){
            0x013c4fd1,
            0x00002392,
            0x00000001,
            0x00000000,
            0x00000000,
            0x00000000,
        },
    .ECCP_P_N = (unsigned int[]){0x7446d879},
    .ECCP_A =
        (unsigned int[]){
            0x00000000,
            0x00000000,
            0x00000000,
            0x00000000,
            0x00000000,
            0x00000000,
        },
    .ECCP_B =
        (unsigned int[]){
            0x00000003,
            0x00000000,
            0x00000000,
            0x00000000,
            0x00000000,
            0x00000000,
        },
};
#endif /* MBEDTLS_ECP_DP_SECP192K1_ENABLED */

#ifdef MBEDTLS_ECP_DP_CURVE25519_ENABLED
static mont_curve_t tlk_mbedtls_x25519_curve_data = {
    .MONT_P_BITLEN = 255,
#ifdef CONFIG_TLK_SOC_SERIES_TL321X
    .n_bitLen = 253,
#endif /* CONFIG_TLK_SOC_SERIES_TL321X */
    .MONT_P =
        (unsigned int[]){
            0xffffffed,
            0xffffffff,
            0xffffffff,
            0xffffffff,
            0xffffffff,
            0xffffffff,
            0xffffffff,
            0x7fffffff,
        },
    .MONT_P_H =
        (unsigned int[]){
            0x000005a4,
            0x00000000,
            0x00000000,
            0x00000000,
            0x00000000,
            0x00000000,
            0x00000000,
            0x00000000,
        },
    .MONT_P_N0 = (unsigned int[]){0x286bca1b},
    .MONT_A24 =
        (unsigned int[]){
            0x0001db41,
            0x00000000,
            0x00000000,
            0x00000000,
            0x00000000,
            0x00000000,
            0x00000000,
            0x00000000,
        },
};
#endif /* MBEDTLS_ECP_DP_CURVE25519_ENABLED */

#ifdef MBEDTLS_ECP_SHORT_WEIERSTRASS_ENABLED
static const struct
{
    mbedtls_ecp_group_id group;
    eccp_curve_t*        curve_data;
} tlk_mbedtls_eccp_curve_linking[] = {
#ifdef MBEDTLS_ECP_DP_SECP256R1_ENABLED
    {
        .group      = MBEDTLS_ECP_DP_SECP256R1,
        .curve_data = &tlk_mbedtls_secp256r1_curve_data,
    },
#endif /* MBEDTLS_ECP_DP_SECP256R1_ENABLED */
#ifdef MBEDTLS_ECP_DP_SECP256K1_ENABLED
    {
        .group      = MBEDTLS_ECP_DP_SECP256K1,
        .curve_data = &tlk_mbedtls_secp256k1_curve_data,
    },
#endif /* MBEDTLS_ECP_DP_SECP256K1_ENABLED */
#ifdef MBEDTLS_ECP_DP_BP256R1_ENABLED
    {
        .group      = MBEDTLS_ECP_DP_BP256R1,
        .curve_data = &tlk_mbedtls_bp256r1_curve_data,
    },
#endif /* MBEDTLS_ECP_DP_BP256R1_ENABLED */
#ifdef MBEDTLS_ECP_DP_SECP224R1_ENABLED
    {
        .group      = MBEDTLS_ECP_DP_SECP224R1,
        .curve_data = &tlk_mbedtls_secp224r1_curve_data,
    },
#endif /* MBEDTLS_ECP_DP_SECP224R1_ENABLED */
#ifdef MBEDTLS_ECP_DP_SECP224K1_ENABLED
    {
        .group      = MBEDTLS_ECP_DP_SECP224K1,
        .curve_data = &tlk_mbedtls_secp224k1_curve_data,
    },
#endif /* MBEDTLS_ECP_DP_SECP224K1_ENABLED */
#ifdef MBEDTLS_ECP_DP_SECP192R1_ENABLED
    {
        .group      = MBEDTLS_ECP_DP_SECP192R1,
        .curve_data = &tlk_mbedtls_secp192r1_curve_data,
    },
#endif /* MBEDTLS_ECP_DP_SECP192R1_ENABLED */
#ifdef MBEDTLS_ECP_DP_SECP192K1_ENABLED
    {
        .group      = MBEDTLS_ECP_DP_SECP192K1,
        .curve_data = &tlk_mbedtls_secp192k1_curve_data,
    },
#endif /* MBEDTLS_ECP_DP_SECP192K1_ENABLED */
};
#endif /* MBEDTLS_ECP_SHORT_WEIERSTRASS_ENABLED */

#ifdef MBEDTLS_ECP_MONTGOMERY_ENABLED
static const struct
{
    mbedtls_ecp_group_id group;
    mont_curve_t*        curve_data;
} tlk_mbedtls_mont_curve_linking[] = {
#ifdef MBEDTLS_ECP_DP_CURVE25519_ENABLED
    {
        .group      = MBEDTLS_ECP_DP_CURVE25519,
        .curve_data = &tlk_mbedtls_x25519_curve_data,
    },
#endif /* MBEDTLS_ECP_DP_CURVE25519_ENABLED */
};
#endif /* MBEDTLS_ECP_MONTGOMERY_ENABLED */

/****************************************************************
 * Private functions declaration
 ****************************************************************/

#ifdef MBEDTLS_ECP_SHORT_WEIERSTRASS_ENABLED
static eccp_curve_t* tlk_mbedtls_eccp_curve_get(const mbedtls_ecp_group* grp)
{
    eccp_curve_t* eccp_curve = NULL;

    for (size_t i = 0;
         i < sizeof(tlk_mbedtls_eccp_curve_linking) / sizeof(tlk_mbedtls_eccp_curve_linking[0]);
         ++i)
    {
        if (tlk_mbedtls_eccp_curve_linking[i].group == grp->id)
        {
            eccp_curve = tlk_mbedtls_eccp_curve_linking[i].curve_data;
            break;
        }
    }
    return eccp_curve;
}
#endif /* MBEDTLS_ECP_SHORT_WEIERSTRASS_ENABLED */

#ifdef MBEDTLS_ECP_MONTGOMERY_ENABLED
static mont_curve_t* tlk_mbedtls_mont_curve_get(const mbedtls_ecp_group* grp)
{
    mont_curve_t* mont_curve = NULL;

    for (size_t i = 0;
         i < sizeof(tlk_mbedtls_mont_curve_linking) / sizeof(tlk_mbedtls_mont_curve_linking[0]);
         ++i)
    {
        if (tlk_mbedtls_mont_curve_linking[i].group == grp->id)
        {
            mont_curve = tlk_mbedtls_mont_curve_linking[i].curve_data;
            break;
        }
    }
    return mont_curve;
}
#endif /* MBEDTLS_ECP_MONTGOMERY_ENABLED */

/****************************************************************
 * Public functions declaration
 ****************************************************************/

int tlk_mbedtls_ecp_check_pubkey(const mbedtls_ecp_group* grp, const mbedtls_ecp_point* pt)
{
    int result = MBEDTLS_ERR_PLATFORM_HW_ACCEL_FAILED;

    do
    {
        if (!grp || !pt)
        {
            result = MBEDTLS_ERR_ECP_BAD_INPUT_DATA;
            break;
        }
        const uint32_t word_len = GET_WORD_LEN(grp->pbits);

        if (word_len > PKE_OPERAND_MAX_WORD_LEN)
        {
            break;
        }
#if defined(MBEDTLS_ECP_SHORT_WEIERSTRASS_ENABLED)
        if (mbedtls_ecp_get_type(grp) == MBEDTLS_ECP_TYPE_SHORT_WEIERSTRASS)
        {
            eccp_curve_t* eccp_curve = tlk_mbedtls_eccp_curve_get(grp);

            if (!eccp_curve)
            {
                break;
            }
            unsigned int Qx[word_len], Qy[word_len];

            (void) mbedtls_mpi_write_binary_le(
                &pt->MBEDTLS_PRIVATE(X), (unsigned char*) Qx, sizeof(Qx));
            (void) mbedtls_mpi_write_binary_le(
                &pt->MBEDTLS_PRIVATE(Y), (unsigned char*) Qy, sizeof(Qy));
            tlk_mbedtls_ecp_lock();
            int r = eccp_pointVerify(eccp_curve, Qx, Qy);

            tlk_mbedtls_ecp_unlock();
            if (r == PKE_SUCCESS)
            {
                result = 0;
            }
            else if (r == PKE_NOT_ON_CURVE)
            {
                result = MBEDTLS_ERR_ECP_INVALID_KEY;
            }
            else
            {
                TLK_LOG_ERROR(mbedtls_pke, "EC check public key error");
            }
            memset(Qx, 0, sizeof(Qx));
            memset(Qy, 0, sizeof(Qy));
        }
#endif /* MBEDTLS_ECP_SHORT_WEIERSTRASS_ENABLED */
    } while (0);
    return result;
}

int tlk_mbedtls_ecp_mul_restartable(mbedtls_ecp_group* grp, mbedtls_ecp_point* R,
                                    const mbedtls_mpi* m, const mbedtls_ecp_point*     P,
                                    int (*f_rng)(void*, unsigned char*, size_t), void* p_rng,
                                    mbedtls_ecp_restart_ctx* rs_ctx)
{
    (void) f_rng;
    (void) p_rng;
    (void) rs_ctx;
    int result = MBEDTLS_ERR_PLATFORM_HW_ACCEL_FAILED;

    do
    {
        if (!grp || !R || !m || !P)
        {
            result = MBEDTLS_ERR_ECP_BAD_INPUT_DATA;
            break;
        }
        if (!mbedtls_mpi_cmp_int(m, 0))
        {
            mbedtls_ecp_set_zero(R);
            result = 0;
            break;
        }

        const uint32_t word_len = GET_WORD_LEN(grp->pbits);

        if (word_len > PKE_OPERAND_MAX_WORD_LEN)
        {
            break;
        }
        unsigned int ms[word_len], Qx[word_len], Qy[word_len];

        (void) mbedtls_mpi_write_binary_le(m, (unsigned char*) ms, sizeof(ms));
        (void) mbedtls_mpi_write_binary_le(&P->MBEDTLS_PRIVATE(X), (unsigned char*) Qx, sizeof(Qx));
        (void) mbedtls_mpi_write_binary_le(&P->MBEDTLS_PRIVATE(Y), (unsigned char*) Qy, sizeof(Qy));
#ifdef MBEDTLS_ECP_SHORT_WEIERSTRASS_ENABLED
        if (mbedtls_ecp_get_type(grp) == MBEDTLS_ECP_TYPE_SHORT_WEIERSTRASS)
        {
            eccp_curve_t* eccp_curve = tlk_mbedtls_eccp_curve_get(grp);

            if (!eccp_curve)
            {
                memset(ms, 0, sizeof(ms));
                memset(Qx, 0, sizeof(Qx));
                memset(Qy, 0, sizeof(Qy));
                break;
            }
            tlk_mbedtls_ecp_lock();
            int r = eccp_pointMul(eccp_curve, ms, Qx, Qy, Qx, Qy);

            tlk_mbedtls_ecp_unlock();
            if (r != PKE_SUCCESS)
            {
                TLK_LOG_ERROR(mbedtls_pke, "EC multiplication error");
                memset(ms, 0, sizeof(ms));
                memset(Qx, 0, sizeof(Qx));
                memset(Qy, 0, sizeof(Qy));
                break;
            }
        }
#endif /* MBEDTLS_ECP_SHORT_WEIERSTRASS_ENABLED */
#ifdef MBEDTLS_ECP_MONTGOMERY_ENABLED
        if (mbedtls_ecp_get_type(grp) == MBEDTLS_ECP_TYPE_MONTGOMERY)
        {
            mont_curve_t* mont_curve = tlk_mbedtls_mont_curve_get(grp);

            if (!mont_curve)
            {
                memset(ms, 0, sizeof(ms));
                memset(Qx, 0, sizeof(Qx));
                memset(Qy, 0, sizeof(Qy));
                break;
            }
            tlk_mbedtls_ecp_lock();
            int r = x25519_pointMul(mont_curve, ms, Qx, Qx);
            tlk_mbedtls_ecp_unlock();
            if (r != PKE_SUCCESS)
            {
                TLK_LOG_ERROR(mbedtls_pke, "EC multiplication error");
                memset(ms, 0, sizeof(ms));
                memset(Qx, 0, sizeof(Qx));
                memset(Qy, 0, sizeof(Qy));
                break;
            }
            memset(Qy, 0, sizeof(Qy));
        }
#endif /* MBEDTLS_ECP_MONTGOMERY_ENABLED */
        (void) mbedtls_mpi_read_binary_le(
            &R->MBEDTLS_PRIVATE(X), (const unsigned char*) Qx, sizeof(Qx));
        (void) mbedtls_mpi_read_binary_le(
            &R->MBEDTLS_PRIVATE(Y), (const unsigned char*) Qy, sizeof(Qy));
        (void) mbedtls_mpi_lset(&R->MBEDTLS_PRIVATE(Z), 1);
        memset(ms, 0, sizeof(ms));
        memset(Qx, 0, sizeof(Qx));
        memset(Qy, 0, sizeof(Qy));
        result = 0;
    } while (0);
    return result;
}

int tlk_mbedtls_ecp_muladd_restartable(mbedtls_ecp_group* grp, mbedtls_ecp_point* R,
                                       const mbedtls_mpi* m, const mbedtls_ecp_point* P,
                                       const mbedtls_mpi* n, const mbedtls_ecp_point* Q,
                                       mbedtls_ecp_restart_ctx* rs_ctx)
{
    (void) rs_ctx;
    int result = MBEDTLS_ERR_PLATFORM_HW_ACCEL_FAILED;

    do
    {
        if (!grp || !R || !m || !P || !n || !Q)
        {
            result = MBEDTLS_ERR_ECP_BAD_INPUT_DATA;
            break;
        }
        if (!mbedtls_mpi_cmp_int(m, 0) && !mbedtls_mpi_cmp_int(n, 0))
        {
            mbedtls_ecp_set_zero(R);
            result = 0;
            break;
        }
        if (!mbedtls_mpi_cmp_int(m, 0))
        {
            result = tlk_mbedtls_ecp_mul_restartable(grp, R, n, Q, NULL, NULL, rs_ctx);
            break;
        }
        if (!mbedtls_mpi_cmp_int(n, 0))
        {
            result = tlk_mbedtls_ecp_mul_restartable(grp, R, m, P, NULL, NULL, rs_ctx);
            break;
        }

        const uint32_t word_len = GET_WORD_LEN(grp->pbits);

        if (word_len > PKE_OPERAND_MAX_WORD_LEN)
        {
            break;
        }
#if defined(MBEDTLS_ECP_SHORT_WEIERSTRASS_ENABLED)
        if (mbedtls_ecp_get_type(grp) == MBEDTLS_ECP_TYPE_SHORT_WEIERSTRASS)
        {
            eccp_curve_t* eccp_curve = tlk_mbedtls_eccp_curve_get(grp);

            if (!eccp_curve)
            {
                break;
            }
            unsigned int ms[word_len], Q1x[word_len], Q1y[word_len], Q2x[word_len], Q2y[word_len];
            int          r;

            (void) mbedtls_mpi_write_binary_le(
                &P->MBEDTLS_PRIVATE(X), (unsigned char*) Q1x, sizeof(Q1x));
            (void) mbedtls_mpi_write_binary_le(
                &P->MBEDTLS_PRIVATE(Y), (unsigned char*) Q1y, sizeof(Q1y));
            (void) mbedtls_mpi_write_binary_le(
                &Q->MBEDTLS_PRIVATE(X), (unsigned char*) Q2x, sizeof(Q2x));
            (void) mbedtls_mpi_write_binary_le(
                &Q->MBEDTLS_PRIVATE(Y), (unsigned char*) Q2y, sizeof(Q2y));

            (void) mbedtls_mpi_write_binary_le(m, (unsigned char*) ms, sizeof(ms));
            tlk_mbedtls_ecp_lock();
            r = eccp_pointMul(eccp_curve, ms, Q1x, Q1y, Q1x, Q1y);
            tlk_mbedtls_ecp_unlock();
            if (r != PKE_SUCCESS)
            {
                TLK_LOG_ERROR(mbedtls_pke, "EC multiplication error");
                memset(ms, 0, sizeof(ms));
                memset(Q1x, 0, sizeof(Q1x));
                memset(Q1y, 0, sizeof(Q1y));
                memset(Q2x, 0, sizeof(Q2x));
                memset(Q2y, 0, sizeof(Q2y));
                break;
            }
            (void) mbedtls_mpi_write_binary_le(n, (unsigned char*) ms, sizeof(ms));
            tlk_mbedtls_ecp_lock();
            r = eccp_pointMul(eccp_curve, ms, Q2x, Q2y, Q2x, Q2y);
            tlk_mbedtls_ecp_unlock();
            if (r != PKE_SUCCESS)
            {
                TLK_LOG_ERROR(mbedtls_pke, "EC multiplication error");
                memset(ms, 0, sizeof(ms));
                memset(Q1x, 0, sizeof(Q1x));
                memset(Q1y, 0, sizeof(Q1y));
                memset(Q2x, 0, sizeof(Q2x));
                memset(Q2y, 0, sizeof(Q2y));
                break;
            }
            tlk_mbedtls_ecp_lock();
            r = eccp_pointAdd(eccp_curve, Q1x, Q1y, Q2x, Q2y, Q1x, Q1y);
            tlk_mbedtls_ecp_unlock();
            if (r != PKE_SUCCESS)
            {
                TLK_LOG_ERROR(mbedtls_pke, "EC summation error");
                memset(ms, 0, sizeof(ms));
                memset(Q1x, 0, sizeof(Q1x));
                memset(Q1y, 0, sizeof(Q1y));
                memset(Q2x, 0, sizeof(Q2x));
                memset(Q2y, 0, sizeof(Q2y));
                break;
            }
            (void) mbedtls_mpi_read_binary_le(
                &R->MBEDTLS_PRIVATE(X), (const unsigned char*) Q1x, sizeof(Q1x));
            (void) mbedtls_mpi_read_binary_le(
                &R->MBEDTLS_PRIVATE(Y), (const unsigned char*) Q1y, sizeof(Q1y));
            (void) mbedtls_mpi_lset(&R->MBEDTLS_PRIVATE(Z), 1);
            memset(ms, 0, sizeof(ms));
            memset(Q1x, 0, sizeof(Q1x));
            memset(Q1y, 0, sizeof(Q1y));
            memset(Q2x, 0, sizeof(Q2x));
            memset(Q2y, 0, sizeof(Q2y));
            result = 0;
        }
#endif /* MBEDTLS_ECP_SHORT_WEIERSTRASS_ENABLED */
    } while (0);
    return result;
}

__attribute__((used, noinline)) static void tlk_mbedtls_ecp_enable(void)
{
#if defined(CONFIG_TLK_SOC_SERIES_TL321X) || defined(CONFIG_TLK_SOC_SERIES_TL721X)
    pke_dig_en();
#endif /* CONFIG_TLK_SOC_SERIES_TL321X || CONFIG_TLK_SOC_SERIES_TL721X */
}

TLK_REGISTER_PRE_INIT(tlk_mbedtls_ecp_enable, TLK_INIT_LEVEL_DRIVER, TLK_INIT_PRIORITY_LOW);
TLK_REGISTER_AFTER_SLEEP(tlk_mbedtls_ecp_enable, TLK_INIT_LEVEL_DRIVER, TLK_INIT_PRIORITY_LOW)

#else

int tlk_mbedtls_ecp_check_pubkey(const mbedtls_ecp_group* grp, const mbedtls_ecp_point* pt)
{
    (void) grp;
    (void) pt;

    return MBEDTLS_ERR_PLATFORM_HW_ACCEL_FAILED;
}

int tlk_mbedtls_ecp_mul_restartable(mbedtls_ecp_group* grp, mbedtls_ecp_point* R,
                                    const mbedtls_mpi* m, const mbedtls_ecp_point*     P,
                                    int (*f_rng)(void*, unsigned char*, size_t), void* p_rng,
                                    mbedtls_ecp_restart_ctx* rs_ctx)
{
    (void) grp;
    (void) R;
    (void) m;
    (void) P;
    (void) f_rng;
    (void) p_rng;
    (void) rs_ctx;

    return MBEDTLS_ERR_PLATFORM_HW_ACCEL_FAILED;
}

int tlk_mbedtls_ecp_muladd_restartable(mbedtls_ecp_group* grp, mbedtls_ecp_point* R,
                                       const mbedtls_mpi* m, const mbedtls_ecp_point* P,
                                       const mbedtls_mpi* n, const mbedtls_ecp_point* Q,
                                       mbedtls_ecp_restart_ctx* rs_ctx)
{
    (void) grp;
    (void) R;
    (void) m;
    (void) P;
    (void) n;
    (void) Q;
    (void) rs_ctx;

    return MBEDTLS_ERR_PLATFORM_HW_ACCEL_FAILED;
}

#endif /* CONFIG_TLK_CRYPTO_PKE */

#endif /* MBEDTLS_ECP_C */
