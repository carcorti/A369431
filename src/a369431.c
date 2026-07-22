/*
 * OEIS A369431 production source (C17).
 *
 * Counts Av_n(1234,1324,1342,2413) by inserting the next maximum.  This is
 * deliberately a fixed program: there is no pattern parser, recurrence
 * generator, scheduler, checkpoint tree, or external executable.  The fixed
 * order-5 frontier gives 66 durable internal recovery units for targets
 * 5..24.  Fixed target-specific operational groups contain those units; each
 * invocation durably closes one human-scale group and exits.
 *
 * Mathematical predicate.  Let q be obtained by inserting its maximum M at
 * position p into an avoiding permutation.  An old forbidden occurrence
 * cannot appear, so every new one contains M.  Since M has rank 4, the four
 * possible cases are exactly:
 *   1234: i<j<k<p and q[i]<q[j]<q[k];
 *   1324: i<j<k<p and q[i]<q[k]<q[j];
 *   1342: i<j<p<k and q[i]<q[k]<q[j];
 *   2413: i<p<j<k and q[j]<q[i]<q[k].
 * incremental_ok implements their negation.  Conversely, every occurrence
 * containing M has one of these forms, hence this test is equivalent to the
 * definition for an avoiding parent.  Deleting the maximum is its unique
 * inverse, proving both completeness and duplicate-freedom of the tree.
 */
#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#ifndef SOURCE_PATH
#define SOURCE_PATH "src/a369431.c"
#endif
#ifndef OUTPUT_DIRECTORY
#define OUTPUT_DIRECTORY "outputs"
#endif

#define SEQUENCE_ID "A369431"
#define KNOWN_MAX_N 20U
#define CAMPAIGN_MAX_N 24U
#define ROOT_ORDER 4U
#define ROOT_COUNT 20U
#define FRONTIER_ORDER 5U
#define SEGMENT_COUNT 66U
#define PERM_CAPACITY CAMPAIGN_MAX_N
#define LINE_CAPACITY 1024U
#define PATH_CAPACITY 512U
#define TEMP_PATH_CAPACITY 544U
#define SHA256_HEX_LENGTH 64U

typedef unsigned __int128 u128;

_Static_assert(KNOWN_MAX_N <= CAMPAIGN_MAX_N, "known terms must fit the campaign domain");
_Static_assert(CAMPAIGN_MAX_N <= UINT8_MAX, "permutation values use uint8_t");
_Static_assert(PERM_CAPACITY >= CAMPAIGN_MAX_N, "workspace must hold the campaign target");

typedef struct {
    u128 leaves;
    u128 nodes;
} Count;

typedef struct {
    bool active;
    unsigned int target;
    unsigned int group;
    unsigned int unit;
    unsigned int group_done;
    unsigned int group_units;
    struct timespec invocation_start;
    struct timespec unit_start;
    struct timespec last_emit;
    u128 next_nodes;
} Progress;

typedef enum {
    SEGMENT_ABSENT,
    SEGMENT_RUNNING,
    SEGMENT_FAILED,
    SEGMENT_COMPLETE
} SegmentState;

typedef enum {
    MANIFEST_OPEN_FAILED,
    MANIFEST_LOCK_FAILED
} ManifestOpenFailure;

typedef struct {
    SegmentState state;
    Count count;
} ManifestSegment;

typedef struct {
    uint32_t h[8];
    uint8_t block[64];
    uint64_t bytes;
    size_t used;
} Sha256;

#define PROGRESS_NODE_STEP ((u128)65536U)
#define PROGRESS_INTERVAL_SECONDS 5.0

static Progress *current_progress = NULL;

/* Lexicographic Av_4(B); these are the 20 mathematical coverage atoms. */
static const uint8_t roots[ROOT_COUNT][ROOT_ORDER] = {
    {1, 2, 4, 3}, {1, 4, 2, 3}, {1, 4, 3, 2}, {2, 1, 3, 4},
    {2, 1, 4, 3}, {2, 3, 1, 4}, {2, 3, 4, 1}, {2, 4, 3, 1},
    {3, 1, 2, 4}, {3, 1, 4, 2}, {3, 2, 1, 4}, {3, 2, 4, 1},
    {3, 4, 1, 2}, {3, 4, 2, 1}, {4, 1, 2, 3}, {4, 1, 3, 2},
    {4, 2, 1, 3}, {4, 2, 3, 1}, {4, 3, 1, 2}, {4, 3, 2, 1}
};

static const uint64_t known_terms[KNOWN_MAX_N + 1U] = {
    1ULL, 1ULL, 2ULL, 6ULL, 20ULL, 66ULL, 214ULL, 688ULL, 2206ULL,
    7070ULL, 22660ULL, 72634ULL, 232830ULL, 746352ULL, 2392486ULL,
    7669286ULL, 24584436ULL, 78807122ULL, 252621702ULL, 809796400ULL,
    2595858574ULL
};

static const uint32_t sha_k[64] = {
    0x428a2f98U,0x71374491U,0xb5c0fbcfU,0xe9b5dba5U,0x3956c25bU,0x59f111f1U,0x923f82a4U,0xab1c5ed5U,
    0xd807aa98U,0x12835b01U,0x243185beU,0x550c7dc3U,0x72be5d74U,0x80deb1feU,0x9bdc06a7U,0xc19bf174U,
    0xe49b69c1U,0xefbe4786U,0x0fc19dc6U,0x240ca1ccU,0x2de92c6fU,0x4a7484aaU,0x5cb0a9dcU,0x76f988daU,
    0x983e5152U,0xa831c66d,0xb00327c8U,0xbf597fc7U,0xc6e00bf3U,0xd5a79147U,0x06ca6351U,0x14292967U,
    0x27b70a85U,0x2e1b2138U,0x4d2c6dfcU,0x53380d13U,0x650a7354U,0x766a0abbU,0x81c2c92eU,0x92722c85U,
    0xa2bfe8a1U,0xa81a664bU,0xc24b8b70U,0xc76c51a3U,0xd192e819U,0xd6990624U,0xf40e3585U,0x106aa070U,
    0x19a4c116U,0x1e376c08U,0x2748774cU,0x34b0bcb5U,0x391c0cb3U,0x4ed8aa4aU,0x5b9cca4fU,0x682e6ff3U,
    0x748f82eeU,0x78a5636fU,0x84c87814U,0x8cc70208U,0x90befffaU,0xa4506cebU,0xbef9a3f7U,0xc67178f2U
};

static uint32_t rotr32(uint32_t x, unsigned int n) { return (x >> n) | (x << (32U - n)); }
static uint32_t load_be32(const uint8_t *p) {
    return ((uint32_t)p[0] << 24U) | ((uint32_t)p[1] << 16U) |
           ((uint32_t)p[2] << 8U) | (uint32_t)p[3];
}
static void store_be32(uint8_t *p, uint32_t x) {
    p[0] = (uint8_t)(x >> 24U); p[1] = (uint8_t)(x >> 16U);
    p[2] = (uint8_t)(x >> 8U); p[3] = (uint8_t)x;
}
static void sha_block(Sha256 *s, const uint8_t block[64]) {
    uint32_t w[64];
    for (unsigned int i = 0; i < 16U; ++i) w[i] = load_be32(block + 4U * i);
    for (unsigned int i = 16; i < 64U; ++i)
        w[i] = (rotr32(w[i-15U], 7U) ^ rotr32(w[i-15U], 18U) ^ (w[i-15U] >> 3U)) + w[i-16U] + w[i-7U]
             + (rotr32(w[i-2U], 17U) ^ rotr32(w[i-2U], 19U) ^ (w[i-2U] >> 10U));
    uint32_t a=s->h[0], b=s->h[1], c=s->h[2], d=s->h[3], e=s->h[4], f=s->h[5], g=s->h[6], h=s->h[7];
    for (unsigned int i = 0; i < 64U; ++i) {
        uint32_t s1 = rotr32(e,6U)^rotr32(e,11U)^rotr32(e,25U);
        uint32_t choose = (e & f) ^ ((~e) & g);
        uint32_t t1 = h + s1 + choose + sha_k[i] + w[i];
        uint32_t s0 = rotr32(a,2U)^rotr32(a,13U)^rotr32(a,22U);
        uint32_t majority = (a & b) ^ (a & c) ^ (b & c);
        uint32_t t2 = s0 + majority;
        h=g; g=f; f=e; e=d+t1; d=c; c=b; b=a; a=t1+t2;
    }
    s->h[0]+=a; s->h[1]+=b; s->h[2]+=c; s->h[3]+=d;
    s->h[4]+=e; s->h[5]+=f; s->h[6]+=g; s->h[7]+=h;
}
static void sha_init(Sha256 *s) {
    static const uint32_t initial[8] = {0x6a09e667U,0xbb67ae85U,0x3c6ef372U,0xa54ff53aU,0x510e527fU,0x9b05688cU,0x1f83d9abU,0x5be0cd19U};
    memcpy(s->h, initial, sizeof initial); s->bytes = 0U; s->used = 0U;
}
static void sha_update(Sha256 *s, const uint8_t *data, size_t length) {
    s->bytes += (uint64_t)length;
    while (length != 0U) {
        size_t take = 64U - s->used;
        if (take > length) take = length;
        memcpy(s->block + s->used, data, take);
        s->used += take; data += take; length -= take;
        if (s->used == 64U) { sha_block(s, s->block); s->used = 0U; }
    }
}
static void sha_final(Sha256 *s, char hex[65]) {
    uint64_t bit_count = s->bytes * UINT64_C(8);
    uint8_t one = 0x80U, zero = 0U, length[8], digest[32];
    sha_update(s, &one, 1U);
    while (s->used != 56U) sha_update(s, &zero, 1U);
    for (unsigned int i = 0; i < 8U; ++i) length[7U-i] = (uint8_t)(bit_count >> (8U*i));
    sha_update(s, length, sizeof length);
    for (unsigned int i = 0; i < 8U; ++i) store_be32(digest + 4U*i, s->h[i]);
    for (unsigned int i = 0; i < sizeof digest; ++i) (void)snprintf(hex + 2U*i, 3U, "%02x", digest[i]);
    hex[64] = '\0';
}
static bool sha256_file(const char *path, char hex[65]) {
    FILE *f = fopen(path, "rb");
    uint8_t buffer[4096];
    Sha256 s;
    if (f == NULL) return false;
    sha_init(&s);
    for (;;) {
        size_t got = fread(buffer, 1U, sizeof buffer, f);
        if (got != 0U) sha_update(&s, buffer, got);
        if (got < sizeof buffer) { if (ferror(f) != 0) { (void)fclose(f); return false; } break; }
    }
    if (fclose(f) != 0) return false;
    sha_final(&s, hex);
    return true;
}
static bool sha256_known_vector(void) {
    static const uint8_t abc[] = {'a','b','c'};
    Sha256 s; char hex[65];
    sha_init(&s); sha_update(&s,abc,sizeof abc); sha_final(&s,hex);
    return strcmp(hex,"ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad")==0;
}

static void u128_text(u128 value, char out[40]) {
    char backward[40]; size_t used = 0U;
    do { backward[used++] = (char)('0' + (value % 10U)); value /= 10U; } while (value != 0U);
    for (size_t i = 0; i < used; ++i) out[i] = backward[used - 1U - i];
    out[used] = '\0';
}
static bool parse_u128(const char *text, u128 *out) {
    u128 v = 0U, ceiling = (u128)-1;
    if (*text == '\0') return false;
    for (; *text != '\0'; ++text) {
        if (*text < '0' || *text > '9' || v > (ceiling - (unsigned)(*text-'0')) / 10U) return false;
        v = v * 10U + (unsigned)(*text - '0');
    }
    *out = v; return true;
}
static bool add_u128(u128 *to, u128 add) {
    if (*to > (u128)-1 - add) return false;
    *to += add; return true;
}
static bool add_count(Count *to, Count add) { return add_u128(&to->leaves, add.leaves) && add_u128(&to->nodes, add.nodes); }
static bool elapsed_seconds_checked(const struct timespec *start, const struct timespec *end, double *seconds) {
    if (start->tv_nsec<0L || start->tv_nsec>=1000000000L || end->tv_nsec<0L || end->tv_nsec>=1000000000L || end->tv_sec<start->tv_sec || (end->tv_sec==start->tv_sec && end->tv_nsec<start->tv_nsec)) return false;
    *seconds=(double)(end->tv_sec-start->tv_sec)+1e-9*(double)(end->tv_nsec-start->tv_nsec);
    return *seconds>=0.0;
}
static void progress_emit(const Progress *progress, const Count *count, const char *phase, const struct timespec *now) {
    char leaves[40], nodes[40];
    double unit_seconds = 0.0, invocation_seconds = 0.0;
    if (!progress->active ||
        !elapsed_seconds_checked(&progress->unit_start, now, &unit_seconds) ||
        !elapsed_seconds_checked(&progress->invocation_start, now, &invocation_seconds)) return;
    u128_text(count->leaves, leaves);
    u128_text(count->nodes, nodes);
    (void)fprintf(stderr,
                  "progress target=%u group=%u unit=%u group_units=%u/%u phase=%s unit_nodes=%s unit_leaves=%s elapsed_unit_s=%.3f elapsed_call_s=%.3f\n",
                  progress->target, progress->group, progress->unit,
                  progress->group_done, progress->group_units, phase,
                  nodes, leaves, unit_seconds, invocation_seconds);
    (void)fflush(stderr);
}
static void progress_begin_unit(Progress *progress, unsigned int target, unsigned int group,
                                unsigned int unit, unsigned int group_done,
                                unsigned int group_units) {
    Count zero = {0U, 0U};
    struct timespec now;
    progress->active = false;
    if (clock_gettime(CLOCK_MONOTONIC, &now) != 0) {
        current_progress = NULL;
        return;
    }
    progress->target = target;
    progress->group = group;
    progress->unit = unit;
    progress->group_done = group_done;
    progress->group_units = group_units;
    progress->unit_start = now;
    progress->last_emit = now;
    progress->next_nodes = PROGRESS_NODE_STEP;
    progress->active = true;
    current_progress = progress;
    progress_emit(progress, &zero, "unit-start", &now);
}
static void progress_maybe_emit(const Count *count) {
    struct timespec now;
    double since_last = 0.0;
    if (current_progress == NULL || !current_progress->active || count->nodes < current_progress->next_nodes) return;
    current_progress->next_nodes = count->nodes > (u128)-1 - PROGRESS_NODE_STEP ? (u128)-1 : count->nodes + PROGRESS_NODE_STEP;
    if (clock_gettime(CLOCK_MONOTONIC, &now) != 0 ||
        !elapsed_seconds_checked(&current_progress->last_emit, &now, &since_last) ||
        since_last < PROGRESS_INTERVAL_SECONDS) return;
    progress_emit(current_progress, count, "running", &now);
    current_progress->last_emit = now;
}
static void progress_finish_unit(const Count *count, bool success) {
    struct timespec now;
    Progress *progress = current_progress;
    if (progress == NULL || !progress->active) return;
    if (clock_gettime(CLOCK_MONOTONIC, &now) == 0)
        progress_emit(progress, count, success ? "unit-complete" : "unit-failed", &now);
    progress->active = false;
    if (current_progress == progress) current_progress = NULL;
}

/* Fixed direct oracle: no encoded pattern table or generic pattern parser. */
static bool direct_avoids(const uint8_t *p, unsigned int n) {
    for (unsigned int i = 0; i + 3U < n; ++i) for (unsigned int j = i+1U; j + 2U < n; ++j)
    for (unsigned int k = j+1U; k + 1U < n; ++k) for (unsigned int l = k+1U; l < n; ++l) {
        uint8_t a=p[i], b=p[j], c=p[k], d=p[l];
        if ((a<b && b<c && c<d) || (a<c && c<b && b<d) ||
            (a<d && d<b && b<c) || (c<a && a<d && d<b)) return false;
    }
    return true;
}
static bool incremental_ok(const uint8_t *q, unsigned int size, unsigned int maximum_position) {
    unsigned int p = maximum_position;
    (void)size;
    for (unsigned int i=0; i<p; ++i) for (unsigned int j=i+1U; j<p; ++j) for (unsigned int k=j+1U; k<p; ++k)
        if ((q[i]<q[j] && q[j]<q[k]) || (q[i]<q[k] && q[k]<q[j])) return false;
    for (unsigned int i=0; i<p; ++i) for (unsigned int j=i+1U; j<p; ++j)
        for (unsigned int k=p+1U; k<size; ++k) if (q[i]<q[k] && q[k]<q[j]) return false;
    for (unsigned int i=0; i<p; ++i) for (unsigned int j=p+1U; j<size; ++j)
        for (unsigned int k=j+1U; k<size; ++k) if (q[j]<q[i] && q[i]<q[k]) return false;
    return true;
}
static void insert_maximum(const uint8_t *parent, unsigned int parent_size, unsigned int position, uint8_t child[PERM_CAPACITY]) {
    for (unsigned int i=0; i<position; ++i) child[i]=parent[i];
    child[position]=(uint8_t)(parent_size+1U);
    for (unsigned int i=position; i<parent_size; ++i) child[i+1U]=parent[i];
}
static bool count_subtree(const uint8_t *p, unsigned int size, unsigned int target, Count *out) {
    if (!add_u128(&out->nodes, 1U)) return false;
    progress_maybe_emit(out);
    if (size == target) return add_u128(&out->leaves, 1U);
    for (unsigned int position=0; position<=size; ++position) {
        uint8_t child[PERM_CAPACITY];
        insert_maximum(p, size, position, child);
        if (incremental_ok(child, size+1U, position) && !count_subtree(child, size+1U, target, out)) return false;
    }
    return true;
}
static bool monolithic_count(unsigned int target, Count *out) {
    const uint8_t start[PERM_CAPACITY] = {1U};
    out->leaves=0U; out->nodes=0U;
    if (target == 0U) { out->leaves=1U; out->nodes=1U; return true; }
    return count_subtree(start, 1U, target, out);
}
static bool root_count(unsigned int target, unsigned int atom, Count *out) {
    out->leaves=0U; out->nodes=0U;
    return count_subtree(roots[atom], ROOT_ORDER, target, out);
}
static unsigned int segment_count_for_target(unsigned int target) {
    return target == ROOT_ORDER ? ROOT_COUNT : SEGMENT_COUNT;
}
/*
 * The campaign recovery frontier is fixed at order 5.  Unit s is the s-th valid
 * insertion of 5 into the lexicographic Av_4(B) roots, ordered by root then
 * insertion position.  Thus the 66 units are a complete, disjoint shallow
 * refinement of the 20 mathematical order-4 atoms; no runtime plan, parser,
 * or scheduler is involved.  Target 4 retains its 20 one-root segments for
 * full known-range replay.  They are internal durable recovery units, not the
 * human stop/go cadence.
 */
static bool segment_prefix(unsigned int target, unsigned int segment,
                           uint8_t prefix[PERM_CAPACITY], unsigned int *atom,
                           unsigned int *position) {
    if (target == ROOT_ORDER) {
        if (segment >= ROOT_COUNT) return false;
        memcpy(prefix, roots[segment], ROOT_ORDER);
        *atom = segment;
        *position = ROOT_ORDER;
        return true;
    }
    if (target < FRONTIER_ORDER || segment >= SEGMENT_COUNT) return false;
    unsigned int found = 0U;
    for (unsigned int a = 0U; a < ROOT_COUNT; ++a) {
        for (unsigned int p = 0U; p <= ROOT_ORDER; ++p) {
            uint8_t child[PERM_CAPACITY];
            insert_maximum(roots[a], ROOT_ORDER, p, child);
            if (!incremental_ok(child, FRONTIER_ORDER, p)) continue;
            if (found++ == segment) {
                memcpy(prefix, child, FRONTIER_ORDER);
                *atom = a;
                *position = p;
                return true;
            }
        }
    }
    return false;
}
static bool segment_count(unsigned int target, unsigned int segment, Count *out) {
    uint8_t prefix[PERM_CAPACITY];
    unsigned int atom, position;
    out->leaves=0U;
    out->nodes=0U;
    if (!segment_prefix(target, segment, prefix, &atom, &position)) return false;
    if (target == ROOT_ORDER) return root_count(target, atom, out);
    (void)position;
    return count_subtree(prefix, target == ROOT_ORDER ? ROOT_ORDER : FRONTIER_ORDER, target, out);
}
/*
 * One invocation closes one operational group.  The group count grows only
 * with the fixed target range, while the fine frontier remains durable inside
 * the group.  The counts below are compile-time fixed human stop/go groups,
 * calibrated from the measured n=20/n=21 target-host throughput to keep
 * official-run invocations in Carlo's requested 30--90 minute range whenever
 * the total target workload is long enough.  Modulo assignment interleaves
 * lexicographic units to avoid putting adjacent root subtrees in the same
 * human-scale run.
 */
static unsigned int group_count_for_target(unsigned int target) {
    if (target <= 21U) return 4U;
    if (target == 22U) return 8U;
    if (target == 23U) return 33U;
    return 66U;
}
static bool unit_in_group(unsigned int target, unsigned int unit, unsigned int group) {
    unsigned int groups=group_count_for_target(target);
    return unit<segment_count_for_target(target) && group<groups && unit%groups==group;
}
static bool next_permutation(uint8_t *p, unsigned int n) {
    if (n < 2U) return false;
    unsigned int i=n-1U; while (i>0U && p[i-1U]>=p[i]) --i;
    if (i==0U) return false;
    unsigned int j=n-1U; while (p[j]<=p[i-1U]) --j;
    uint8_t t=p[i-1U]; p[i-1U]=p[j]; p[j]=t;
    for (unsigned int a=i,b=n-1U; a<b; ++a,--b) { t=p[a]; p[a]=p[b]; p[b]=t; }
    return true;
}
static bool verify_roots(void) {
    uint8_t p[ROOT_ORDER] = {1U,2U,3U,4U}; unsigned int found=0U;
    do {
        if (direct_avoids(p, ROOT_ORDER)) {
            if (found>=ROOT_COUNT || memcmp(p, roots[found], ROOT_ORDER)!=0) return false;
            ++found;
        }
    } while (next_permutation(p, ROOT_ORDER));
    return found==ROOT_COUNT;
}
static bool verify_segments(void) {
    unsigned int per_atom[ROOT_COUNT] = {0U};
    for (unsigned int s = 0U; s < SEGMENT_COUNT; ++s) {
        uint8_t prefix[PERM_CAPACITY];
        unsigned int atom, position;
        if (!segment_prefix(FRONTIER_ORDER, s, prefix, &atom, &position) ||
            atom >= ROOT_COUNT || position > ROOT_ORDER ||
            !direct_avoids(prefix, FRONTIER_ORDER)) return false;
        ++per_atom[atom];
    }
    for (unsigned int a = 0U; a < ROOT_COUNT; ++a) {
        unsigned int expected = 0U;
        for (unsigned int p = 0U; p <= ROOT_ORDER; ++p) {
            uint8_t child[PERM_CAPACITY];
            insert_maximum(roots[a], ROOT_ORDER, p, child);
            if (incremental_ok(child, FRONTIER_ORDER, p)) ++expected;
        }
        if (per_atom[a] != expected) return false;
    }
    { uint8_t prefix[PERM_CAPACITY]; unsigned int atom, position;
      return !segment_prefix(FRONTIER_ORDER, SEGMENT_COUNT, prefix, &atom, &position); }
}
static bool verify_groups(void) {
    static const unsigned int targets[] = {4U,20U,21U,22U,23U,24U};
    static const unsigned int expected_groups[] = {4U,4U,4U,8U,33U,66U};
    for (size_t i=0U; i<sizeof targets/sizeof targets[0]; ++i) {
        unsigned int target=targets[i], groups=group_count_for_target(target);
        if (groups!=expected_groups[i] || groups==0U || groups>segment_count_for_target(target)) return false;
        for (unsigned int unit=0U; unit<segment_count_for_target(target); ++unit) {
            unsigned int memberships=0U;
            for (unsigned int group=0U; group<groups; ++group)
                if (unit_in_group(target,unit,group)) ++memberships;
            if (memberships!=1U) return false;
        }
    }
    return true;
}
static bool verify_incremental_oracle(unsigned int maximum_parent_size) {
    for (unsigned int n=0U; n<=maximum_parent_size; ++n) {
        uint8_t p[PERM_CAPACITY]; for (unsigned int i=0; i<n; ++i) p[i]=(uint8_t)(i+1U);
        do {
            if (direct_avoids(p,n)) for (unsigned int position=0; position<=n; ++position) {
                uint8_t q[PERM_CAPACITY]; insert_maximum(p,n,position,q);
                if (incremental_ok(q,n+1U,position)!=direct_avoids(q,n+1U)) return false;
            }
        } while (next_permutation(p,n));
    }
    return true;
}
static bool direct_oracle_replay(unsigned int maximum_n) {
    if (maximum_n > KNOWN_MAX_N) return false;
    for (unsigned int n=0U; n<=maximum_n; ++n) {
        uint8_t p[PERM_CAPACITY]; unsigned int total=0U;
        for (unsigned int i=0; i<n; ++i) p[i]=(uint8_t)(i+1U);
        do { if (direct_avoids(p,n)) ++total; } while (next_permutation(p,n));
        if ((uint64_t)total != known_terms[n]) return false;
    }
    return true;
}
static bool recurrence_audit(void) {
    for (unsigned int n=4U; n<=KNOWN_MAX_N; ++n) {
        uint64_t r=5U*known_terms[n-1U]-7U*known_terms[n-2U]+4U*known_terms[n-3U];
        if (r!=known_terms[n]) return false;
    }
    return true;
}
static bool validate_counts(unsigned int maximum_n) {
    if (maximum_n > KNOWN_MAX_N) return false;
    for (unsigned int n=0U; n<=maximum_n; ++n) {
        Count c;
        if (!monolithic_count(n,&c) || c.leaves!=(u128)known_terms[n]) return false;
        if (n>=ROOT_ORDER) {
            Count segmented={0U,0U};
            for (unsigned int s=0U; s<segment_count_for_target(n); ++s) { Count x; if (!segment_count(n,s,&x) || !add_count(&segmented,x)) return false; }
            if (segmented.leaves!=c.leaves) return false;
        }
    }
    return true;
}

static const char manifest_header[]="sequence\ttarget\tsegment\tcoverage\tstatus\tleaves\tnodes\telapsed_s\tsource_sha256\tbinary_sha256\tepoch\tstop_reason\n";
static const char aggregate_header[]="sequence\ttarget\tcoverage\tleaves\tnodes\tsource_sha256\tbinary_sha256\tmanifest_sha256\tmethod\trecurrence_status\n";

static bool coverage_text(unsigned int target, unsigned int segment, char out[32]) {
    uint8_t prefix[PERM_CAPACITY];
    unsigned int atom, position;
    int n;
    if (!segment_prefix(target, segment, prefix, &atom, &position)) return false;
    (void)prefix;
    if (target == ROOT_ORDER) n=snprintf(out,32U,"root=%u",atom);
    else n=snprintf(out,32U,"root=%u,insert5=%u",atom,position);
    return n>=0 && (size_t)n<32U;
}
static bool path_for(char *out, size_t length, unsigned int target, const char *suffix) {
    int n;
    if (length==0U) return false;
    n=snprintf(out,length,"%s/a369431_n%02u.%s",OUTPUT_DIRECTORY,target,suffix);
    return n>=0 && (size_t)n<length;
}
static bool write_all(int fd, const char *at, size_t left) {
    while (left!=0U) {
        ssize_t put=write(fd,at,left);
        if (put<0 && errno==EINTR) continue;
        if (put<=0) return false;
        at+=(size_t)put; left-=(size_t)put;
    }
    return true;
}
static bool fsync_retry(int fd) {
    for (;;) { if (fsync(fd)==0) return true; if (errno!=EINTR) return false; }
}
static bool sync_and_close(int fd) {
    bool synced=fsync_retry(fd);
    bool closed=close(fd)==0;
    return synced && closed;
}
static bool stream_read_and_close(FILE *stream) {
    bool read_ok=ferror(stream)==0;
    bool close_ok=fclose(stream)==0;
    return read_ok && close_ok;
}
static const char *lock_stop_reason(int error_code) {
    return (error_code==EWOULDBLOCK || error_code==EAGAIN) ? "manifest-busy" : "manifest-lock-failed";
}
static bool fsync_output_directory(void) {
    int fd=open(OUTPUT_DIRECTORY,O_RDONLY|O_DIRECTORY);
    if (fd<0) return false;
    return sync_and_close(fd);
}
static bool open_manifest_locked(const char *path, int *fd_out, ManifestOpenFailure *failure) {
    int fd=open(path,O_RDWR|O_CREAT,0644);
    struct stat st;
    *failure=MANIFEST_OPEN_FAILED;
    if (fd<0) return false;
    if (flock(fd,LOCK_EX|LOCK_NB)!=0) { int saved_errno=errno; (void)close(fd); errno=saved_errno; *failure=MANIFEST_LOCK_FAILED; return false; }
    if (fstat(fd,&st)!=0) { (void)close(fd); return false; }
    if (st.st_size==0) {
        if (!write_all(fd,manifest_header,sizeof manifest_header-1U) || !fsync_retry(fd) || !fsync_output_directory()) { (void)close(fd); return false; }
    }
    *fd_out=fd;
    return true;
}
static bool append_manifest(int fd, unsigned int target, unsigned int segment, const char *status, Count count, double seconds, const char *source_hash, const char *binary_hash, const char *reason) {
    char leaves[40],nodes[40],coverage[32],line[LINE_CAPACITY];
    time_t now=time(NULL);
    int n;
    if (now==(time_t)-1 || !coverage_text(target,segment,coverage)) return false;
    u128_text(count.leaves,leaves); u128_text(count.nodes,nodes);
    n=snprintf(line,sizeof line,"%s\t%u\t%u\t%s\t%s\t%s\t%s\t%.6f\t%s\t%s\t%lld\t%s\n",SEQUENCE_ID,target,segment,coverage,status,leaves,nodes,seconds,source_hash,binary_hash,(long long)now,reason);
    if (n<0 || (size_t)n>=sizeof line || lseek(fd,0,SEEK_END)<0) return false;
    return write_all(fd,line,(size_t)n) && fsync_retry(fd);
}
static bool parse_unsigned(const char *text, unsigned int *out) {
    u128 value;
    if (!parse_u128(text,&value) || value>(u128)UINT32_MAX) return false;
    *out=(unsigned int)value;
    return true;
}
static bool exact_sha256(const char *text) {
    if (strlen(text)!=SHA256_HEX_LENGTH) return false;
    for (size_t i=0U; i<SHA256_HEX_LENGTH; ++i)
        if (!((text[i]>='0' && text[i]<='9') || (text[i]>='a' && text[i]<='f'))) return false;
    return true;
}
static bool elapsed_text_ok(const char *text) {
    size_t i=0U, after=0U;
    while (text[i]>='0' && text[i]<='9') ++i;
    if (i==0U || text[i++]!='.') return false;
    while (text[i]>='0' && text[i]<='9') { ++i; ++after; }
    return after==6U && text[i]=='\0';
}
static bool split_tsv_12(char *line, char *fields[12]) {
    unsigned int count=0U;
    fields[count++]=line;
    for (char *p=line; *p!='\0'; ++p) if (*p=='\t') {
        *p='\0';
        if (count==12U) return false;
        fields[count++]=p+1;
    }
    return count==12U;
}
static bool read_manifest(const char *path, unsigned int target, const char *source_hash, const char *binary_hash, ManifestSegment states[SEGMENT_COUNT]) {
    FILE *f=fopen(path,"r"); char line[LINE_CAPACITY];
    unsigned int segment_limit=segment_count_for_target(target);
    if (f==NULL || !exact_sha256(source_hash) || !exact_sha256(binary_hash)) { if (f!=NULL) (void)fclose(f); return false; }
    for (unsigned int s=0U; s<SEGMENT_COUNT; ++s) { states[s].state=SEGMENT_ABSENT; states[s].count=(Count){0U,0U}; }
    if (fgets(line,sizeof line,f)==NULL || strcmp(line,manifest_header)!=0) { (void)fclose(f); return false; }
    while (fgets(line,sizeof line,f)!=NULL) {
        char *fields[12], coverage[32]; unsigned int file_target,segment; u128 epoch; Count count;
        size_t length=strlen(line);
        if (length==0U || line[length-1U]!='\n' || strchr(line,'\r')!=NULL) { (void)fclose(f); return false; }
        line[length-1U]='\0';
        if (!split_tsv_12(line,fields) || !parse_unsigned(fields[1],&file_target) || !parse_unsigned(fields[2],&segment) || file_target!=target || segment>=segment_limit || !coverage_text(target,segment,coverage) || strcmp(fields[0],SEQUENCE_ID)!=0 || strcmp(fields[3],coverage)!=0 || strcmp(fields[8],source_hash)!=0 || strcmp(fields[9],binary_hash)!=0 || !exact_sha256(fields[8]) || !exact_sha256(fields[9]) || !parse_u128(fields[5],&count.leaves) || !parse_u128(fields[6],&count.nodes) || !elapsed_text_ok(fields[7]) || !parse_u128(fields[10],&epoch)) { (void)fclose(f); return false; }
        if (strcmp(fields[4],"running")==0) {
            bool restart=strcmp(fields[11],"restarted")==0;
            if ((states[segment].state==SEGMENT_ABSENT && strcmp(fields[11],"started")!=0) || (states[segment].state==SEGMENT_FAILED && strcmp(fields[11],"started")!=0) || (states[segment].state==SEGMENT_RUNNING && !restart) || states[segment].state==SEGMENT_COMPLETE || count.leaves!=0U || count.nodes!=0U || strcmp(fields[7],"0.000000")!=0) { (void)fclose(f); return false; }
            states[segment].state=SEGMENT_RUNNING;
        } else if (strcmp(fields[4],"failed")==0) {
            if (states[segment].state!=SEGMENT_RUNNING || strcmp(fields[11],"counter-overflow")!=0) { (void)fclose(f); return false; }
            states[segment].state=SEGMENT_FAILED; states[segment].count=count;
        } else if (strcmp(fields[4],"complete")==0) {
            if (states[segment].state!=SEGMENT_RUNNING || strcmp(fields[11],"completed")!=0 || count.leaves==0U || count.nodes==0U) { (void)fclose(f); return false; }
            states[segment].state=SEGMENT_COMPLETE; states[segment].count=count;
        } else { (void)fclose(f); return false; }
    }
    return stream_read_and_close(f);
}
static bool partial_total(const ManifestSegment states[SEGMENT_COUNT], unsigned int target, Count *total, unsigned int *complete_segments, unsigned int *complete_atoms) {
    unsigned int expected[ROOT_COUNT] = {0U}, completed[ROOT_COUNT] = {0U};
    unsigned int segment_limit=segment_count_for_target(target);
    total->leaves=0U;
    total->nodes=0U;
    *complete_segments=0U;
    *complete_atoms=0U;
    for (unsigned int s=0U; s<segment_limit; ++s) {
        uint8_t prefix[PERM_CAPACITY];
        unsigned int atom, position;
        if (!segment_prefix(target,s,prefix,&atom,&position)) return false;
        (void)prefix;
        (void)position;
        ++expected[atom];
        if (states[s].state==SEGMENT_COMPLETE) {
            if (!add_count(total,states[s].count)) return false;
            ++completed[atom];
            ++*complete_segments;
        }
    }
    for (unsigned int atom=0U; atom<ROOT_COUNT; ++atom)
        if (completed[atom]==expected[atom]) ++*complete_atoms;
    return true;
}
static bool completed_total(const ManifestSegment states[SEGMENT_COUNT], unsigned int target, Count *total) {
    unsigned int complete_segments, complete_atoms;
    return partial_total(states,target,total,&complete_segments,&complete_atoms) &&
           complete_segments==segment_count_for_target(target) && complete_atoms==ROOT_COUNT;
}
static bool group_total(const ManifestSegment states[SEGMENT_COUNT], unsigned int target, unsigned int group, Count *total, unsigned int *complete_units, unsigned int *unit_total) {
    unsigned int groups=group_count_for_target(target);
    if (group>=groups) return false;
    total->leaves=0U;
    total->nodes=0U;
    *complete_units=0U;
    *unit_total=0U;
    for (unsigned int unit=0U; unit<segment_count_for_target(target); ++unit) {
        if (!unit_in_group(target,unit,group)) continue;
        ++*unit_total;
        if (states[unit].state==SEGMENT_COMPLETE) {
            if (!add_count(total,states[unit].count)) return false;
            ++*complete_units;
        }
    }
    return *unit_total!=0U;
}
static bool group_is_complete(const ManifestSegment states[SEGMENT_COUNT], unsigned int target, unsigned int group) {
    Count total;
    unsigned int complete_units, unit_total;
    return group_total(states,target,group,&total,&complete_units,&unit_total) && complete_units==unit_total;
}
static bool next_incomplete_group(const ManifestSegment states[SEGMENT_COUNT], unsigned int target, unsigned int *group) {
    for (unsigned int g=0U; g<group_count_for_target(target); ++g)
        if (!group_is_complete(states,target,g)) { *group=g; return true; }
    return false;
}
static bool aggregate_text(char line[LINE_CAPACITY], size_t *length, unsigned int target, Count total, const char *source_hash, const char *binary_hash, const char *manifest_hash) {
    char leaves[40],nodes[40],coverage[64]; int n;
    u128_text(total.leaves,leaves); u128_text(total.nodes,nodes);
    n=snprintf(coverage,sizeof coverage,"groups=%u/%u;units=%u/%u;atoms=20/20",group_count_for_target(target),group_count_for_target(target),segment_count_for_target(target),segment_count_for_target(target));
    if (n<0 || (size_t)n>=sizeof coverage) return false;
    n=snprintf(line,LINE_CAPACITY,"%s%s\t%u\t%s\t%s\t%s\t%s\t%s\t%s\tmaximum-insertion\tunproved-post-hoc-checksum-only\n",aggregate_header,SEQUENCE_ID,target,coverage,leaves,nodes,source_hash,binary_hash,manifest_hash);
    if (n<0 || (size_t)n>=LINE_CAPACITY) return false;
    *length=(size_t)n; return true;
}
static bool aggregate_matches(const char *path, const char *expected, size_t expected_length, bool *exists) {
    int fd=open(path,O_RDONLY); struct stat st; char actual[LINE_CAPACITY]={0}; size_t got=0U;
    *exists=false;
    if (expected_length>sizeof actual) return false;
    if (fd<0) return errno==ENOENT;
    *exists=true;
    if (fstat(fd,&st)!=0 || !S_ISREG(st.st_mode) || st.st_size<0 || (uintmax_t)st.st_size!=expected_length) { (void)close(fd); return false; }
    while (got<expected_length) { ssize_t n=read(fd,actual+got,expected_length-got); if (n<0 && errno==EINTR) continue; if (n<=0) { (void)close(fd); return false; } got+=(size_t)n; }
    return close(fd)==0 && memcmp(actual,expected,expected_length)==0;
}
static bool publish_aggregate(const char *path, const char *expected, size_t expected_length) {
    char temporary[TEMP_PATH_CAPACITY]; bool exists; int fd; int n=snprintf(temporary,sizeof temporary,"%s.tmp",path);
    if (n<0 || (size_t)n>=sizeof temporary || !aggregate_matches(path,expected,expected_length,&exists)) return false;
    if (exists) return true;
    (void)unlink(temporary);
    fd=open(temporary,O_WRONLY|O_CREAT|O_EXCL,0644);
    if (fd<0) return false;
    { bool wrote=write_all(fd,expected,expected_length); bool closed=sync_and_close(fd);
      if (!wrote || !closed) { (void)unlink(temporary); return false; } }
    if (link(temporary,path)!=0) {
        int link_errno=errno;
        bool matches=link_errno==EEXIST && aggregate_matches(path,expected,expected_length,&exists);
        (void)unlink(temporary);
        errno=link_errno;
        return matches && exists;
    }
    if (unlink(temporary)!=0) return false;
    return fsync_output_directory();
}
static void print_count(const char *label, Count count) {
    char leaves[40],nodes[40]; u128_text(count.leaves,leaves); u128_text(count.nodes,nodes);
    (void)printf("%s leaves=%s nodes=%s\n",label,leaves,nodes);
}
static bool manifest_fixture(const char *text, bool expected) {
    char path[]="/tmp/a369431_manifest_XXXXXX";
    static const char zero_hash[]="0000000000000000000000000000000000000000000000000000000000000000";
    ManifestSegment states[SEGMENT_COUNT];
    int fd=mkstemp(path);
    bool wrote, closed, result;
    if (fd<0) return false;
    wrote=write_all(fd,text,strlen(text)); closed=close(fd)==0;
    result=wrote && closed && read_manifest(path,12U,zero_hash,zero_hash,states)==expected;
    (void)unlink(path);
    return result;
}
static bool interrupted_group_fixture(void) {
    static const char zero_hash[]="0000000000000000000000000000000000000000000000000000000000000000";
    char path[]="/tmp/a369431_group_XXXXXX", coverage0[32], coverage4[32], text[LINE_CAPACITY];
    ManifestSegment states[SEGMENT_COUNT];
    unsigned int group;
    int fd, n;
    bool wrote, closed, result;
    if (!coverage_text(12U,0U,coverage0) || !coverage_text(12U,4U,coverage4)) return false;
    n=snprintf(text,sizeof text,"%sA369431\t12\t0\t%s\trunning\t0\t0\t0.000000\t%s\t%s\t1\tstarted\nA369431\t12\t0\t%s\tcomplete\t1\t2\t0.000001\t%s\t%s\t2\tcompleted\nA369431\t12\t4\t%s\trunning\t0\t0\t0.000000\t%s\t%s\t3\tstarted\n",manifest_header,coverage0,zero_hash,zero_hash,coverage0,zero_hash,zero_hash,coverage4,zero_hash,zero_hash);
    if (n<0 || (size_t)n>=sizeof text) return false;
    fd=mkstemp(path);
    if (fd<0) return false;
    wrote=write_all(fd,text,(size_t)n);
    closed=close(fd)==0;
    result=wrote && closed && read_manifest(path,12U,zero_hash,zero_hash,states) && !group_is_complete(states,12U,0U) && next_incomplete_group(states,12U,&group) && group==0U;
    (void)unlink(path);
    return result;
}
static bool aggregate_fixture(void) {
    static const char exact[]="aggregate fixture\n";
    static const char stale[]="aggregate stale\n";
    char path[]="/tmp/a369431_aggregate_XXXXXX";
    bool exists, wrote, closed, good;
    int fd=mkstemp(path);
    if (fd<0) return false;
    wrote=write_all(fd,exact,sizeof exact-1U); closed=close(fd)==0;
    good=wrote && closed && aggregate_matches(path,exact,sizeof exact-1U,&exists) && exists && !aggregate_matches(path,stale,sizeof stale-1U,&exists) && !aggregate_matches(path,exact,LINE_CAPACITY+1U,&exists);
    (void)unlink(path);
    return good && aggregate_matches(path,exact,sizeof exact-1U,&exists) && !exists;
}
static bool partial_total_selftest(void) {
    ManifestSegment states[SEGMENT_COUNT] = {{SEGMENT_ABSENT,{0U,0U}}};
    ManifestSegment group_states[SEGMENT_COUNT] = {{SEGMENT_ABSENT,{0U,0U}}};
    ManifestSegment interrupted[SEGMENT_COUNT] = {{SEGMENT_ABSENT,{0U,0U}}};
    Count total;
    unsigned int segments, atoms;
    if (!partial_total(states,FRONTIER_ORDER,&total,&segments,&atoms) || total.leaves!=0U || total.nodes!=0U || segments!=0U || atoms!=0U || completed_total(states,FRONTIER_ORDER,&total)) return false;
    states[0].state=SEGMENT_COMPLETE;
    states[0].count=(Count){1U,2U};
    if (!partial_total(states,FRONTIER_ORDER,&total,&segments,&atoms) || total.leaves!=1U || total.nodes!=2U || segments!=1U || atoms>=ROOT_COUNT) return false;
    states[1].state=SEGMENT_COMPLETE;
    states[1].count=(Count){3U,4U};
    if (!partial_total(states,FRONTIER_ORDER,&total,&segments,&atoms) || total.leaves!=4U || total.nodes!=6U || segments!=2U || atoms>=ROOT_COUNT) return false;
    for (unsigned int s=0U; s<SEGMENT_COUNT; ++s) {
        states[s].state=SEGMENT_COMPLETE;
        states[s].count=(Count){1U,2U};
    }
    if (!partial_total(states,FRONTIER_ORDER,&total,&segments,&atoms) || total.leaves!=(u128)SEGMENT_COUNT || total.nodes!=(u128)(2U*SEGMENT_COUNT) || segments!=SEGMENT_COUNT || atoms!=ROOT_COUNT || !completed_total(states,FRONTIER_ORDER,&total)) return false;
    for (unsigned int unit=0U; unit<segment_count_for_target(20U); ++unit) if (unit_in_group(20U,unit,0U)) {
        group_states[unit].state=SEGMENT_COMPLETE;
        group_states[unit].count=(Count){1U,2U};
    }
    { unsigned int group, complete_units, unit_total;
      if (!group_total(group_states,20U,0U,&total,&complete_units,&unit_total) || complete_units!=unit_total || unit_total!=17U || total.leaves!=17U || total.nodes!=34U || !group_is_complete(group_states,20U,0U) || !next_incomplete_group(group_states,20U,&group) || group!=1U) return false; }
    interrupted[0].state=SEGMENT_COMPLETE;
    interrupted[0].count=(Count){1U,2U};
    interrupted[4].state=SEGMENT_RUNNING;
    { unsigned int group, complete_units, unit_total;
      if (!group_total(interrupted,20U,0U,&total,&complete_units,&unit_total) || complete_units!=1U || unit_total!=17U || group_is_complete(interrupted,20U,0U) || !next_incomplete_group(interrupted,20U,&group) || group!=0U) return false; }
    return segment_count_for_target(ROOT_ORDER)==ROOT_COUNT && segment_count_for_target(FRONTIER_ORDER)==SEGMENT_COUNT && segment_count_for_target(CAMPAIGN_MAX_N)==SEGMENT_COUNT;
}
static bool low_level_selftest(void) {
    struct timespec start={1,0L}, end={1,1L}, backwards={0,999999999L}, invalid={1,1000000000L};
    double seconds=0.0;
    return elapsed_seconds_checked(&start,&end,&seconds) && seconds>0.0 && !elapsed_seconds_checked(&start,&backwards,&seconds) && !elapsed_seconds_checked(&invalid,&end,&seconds) && strcmp(lock_stop_reason(EWOULDBLOCK),"manifest-busy")==0 && strcmp(lock_stop_reason(EAGAIN),"manifest-busy")==0 && strcmp(lock_stop_reason(EIO),"manifest-lock-failed")==0;
}
static bool persistence_selftest(void) {
    static const char zero_hash[]="0000000000000000000000000000000000000000000000000000000000000000";
    char good[LINE_CAPACITY], malformed[LINE_CAPACITY], foreign[LINE_CAPACITY], truncated[LINE_CAPACITY], short_path[8], coverage[32];
    int n;
    if (!coverage_text(12U,0U,coverage)) return false;
    n=snprintf(good,sizeof good,"%sA369431\t12\t0\t%s\trunning\t0\t0\t0.000000\t%s\t%s\t1\tstarted\nA369431\t12\t0\t%s\tcomplete\t1\t1\t0.000001\t%s\t%s\t2\tcompleted\n",manifest_header,coverage,zero_hash,zero_hash,coverage,zero_hash,zero_hash);
    if (n<0 || (size_t)n>=sizeof good || !manifest_fixture(good,true)) return false;
    n=snprintf(malformed,sizeof malformed,"%sA369431\t12\t0\t%s\tcomplete\t1\t1\t0.000001\t%s\t%s\t2\n",manifest_header,coverage,zero_hash,zero_hash);
    if (n<0 || (size_t)n>=sizeof malformed || !manifest_fixture(malformed,false)) return false;
    n=snprintf(foreign,sizeof foreign,"%sA369431\t13\t0\t%s\trunning\t0\t0\t0.000000\t%s\t%s\t1\tstarted\n",manifest_header,coverage,zero_hash,zero_hash);
    if (n<0 || (size_t)n>=sizeof foreign || !manifest_fixture(foreign,false)) return false;
    n=snprintf(truncated,sizeof truncated,"%sA369431\t12\t0\t%s\trunning\t0\t0\t0.000000\t%s\t%s\t1\tstarted",manifest_header,coverage,zero_hash,zero_hash);
    return n>=0 && (size_t)n<sizeof truncated && manifest_fixture(truncated,false) && interrupted_group_fixture() && aggregate_fixture() && partial_total_selftest() && low_level_selftest() && !parse_unsigned(" 12",&(unsigned int){0U}) && !parse_unsigned("+12",&(unsigned int){0U}) && !parse_unsigned("12x",&(unsigned int){0U}) && parse_unsigned("12",&(unsigned int){0U}) && !path_for(short_path,sizeof short_path,12U,"manifest.tsv");
}
static int selftest(void) {
    const unsigned int replay_max=12U, oracle_max=9U;
    bool ok=sha256_known_vector() && verify_roots() && verify_segments() && verify_groups() && verify_incremental_oracle(oracle_max-1U) && direct_oracle_replay(oracle_max) && validate_counts(replay_max) && recurrence_audit() && persistence_selftest();
    (void)printf("A369431 selftest: roots=%s order-5-units=%u incremental-via-n=%u direct-oracle-via-n=%u insertion-replay-via-n=%u recurrence-audit(n=4..20, checksum-only)=%s\n",ok?"ok":"FAIL",SEGMENT_COUNT,oracle_max,oracle_max,replay_max,recurrence_audit()?"agree":"FAIL");
    return ok?0:1;
}
static bool complete_unit_locked(int fd, unsigned int target, unsigned int unit, const char *source_hash, const char *binary_hash, ManifestSegment states[SEGMENT_COUNT], double *seconds) {
    Count zero={0U,0U}, result;
    struct timespec start,end;
    const char *start_reason=states[unit].state==SEGMENT_RUNNING?"restarted":"started";
    if (!append_manifest(fd,target,unit,"running",zero,0.0,source_hash,binary_hash,start_reason)) {
        progress_finish_unit(&zero, false);
        return false;
    }
    if (clock_gettime(CLOCK_MONOTONIC,&start)!=0) {
        progress_finish_unit(&zero, false);
        return false;
    }
    if (!segment_count(target,unit,&result)) {
        if (clock_gettime(CLOCK_MONOTONIC,&end)==0 && elapsed_seconds_checked(&start,&end,seconds))
            (void)append_manifest(fd,target,unit,"failed",result,*seconds,source_hash,binary_hash,"counter-overflow");
        progress_finish_unit(&result, false);
        return false;
    }
    if (clock_gettime(CLOCK_MONOTONIC,&end)!=0 || !elapsed_seconds_checked(&start,&end,seconds)) {
        progress_finish_unit(&result, false);
        return false;
    }
    if (!append_manifest(fd,target,unit,"complete",result,*seconds,source_hash,binary_hash,"completed")) {
        progress_finish_unit(&result, false);
        return false;
    }
    states[unit].state=SEGMENT_COMPLETE;
    states[unit].count=result;
    progress_finish_unit(&result, true);
    return true;
}
static int run_group(unsigned int target, bool choose_next, unsigned int group, const char *binary_path) {
    char manifest[PATH_CAPACITY],aggregate[PATH_CAPACITY],source_hash[65],binary_hash[65];
    ManifestSegment states[SEGMENT_COUNT];
    int lockfd;
    ManifestOpenFailure manifest_failure;
    unsigned int groups=group_count_for_target(target);
    if (target<ROOT_ORDER || target>CAMPAIGN_MAX_N || (!choose_next && group>=groups)) {
        (void)fprintf(stderr,"run accepts target 4..%u and group 0..%u, or next.\n",CAMPAIGN_MAX_N,groups-1U);
        return 2;
    }
    if (!sha256_file(SOURCE_PATH,source_hash) || !sha256_file(binary_path,binary_hash) || !path_for(manifest,sizeof manifest,target,"manifest.tsv") || !path_for(aggregate,sizeof aggregate,target,"aggregate.tsv")) { (void)fprintf(stderr,"cannot establish provenance paths/hashes\n"); return 1; }
    if (!open_manifest_locked(manifest,&lockfd,&manifest_failure)) {
        int saved_errno=errno;
        const char *reason=manifest_failure==MANIFEST_LOCK_FAILED ? lock_stop_reason(saved_errno) : "manifest-open-failed";
        (void)fprintf(stderr,"target=%u stop_reason=%s: %s\n",target,reason,strerror(saved_errno));
        return 1;
    }
    if (!read_manifest(manifest,target,source_hash,binary_hash,states)) { (void)close(lockfd); (void)fprintf(stderr,"target=%u stop_reason=manifest-conflict\n",target); return 1; }
    if (choose_next && !next_incomplete_group(states,target,&group)) group=0U;
    if (group_is_complete(states,target,group)) {
        Count partial;
        unsigned int complete_units, complete_atoms;
        if (!partial_total(states,target,&partial,&complete_units,&complete_atoms)) { (void)close(lockfd); (void)fprintf(stderr,"target=%u stop_reason=partial-total-overflow\n",target); return 1; }
        if (completed_total(states,target,&partial)) {
            char manifest_hash[65],expected[LINE_CAPACITY]; size_t expected_length;
            if (!sha256_file(manifest,manifest_hash) || !aggregate_text(expected,&expected_length,target,partial,source_hash,binary_hash,manifest_hash) || !publish_aggregate(aggregate,expected,expected_length)) { (void)close(lockfd); (void)fprintf(stderr,"target=%u stop_reason=aggregate-conflict-or-publication-failed\n",target); return 1; }
            (void)printf("target=%u group=%u groups=%u/%u units=%u/%u atoms=%u/20 aggregate=%s stop_reason=no-clobber\n",target,group,groups,groups,complete_units,segment_count_for_target(target),complete_atoms,aggregate);
    } else (void)printf("target=%u group=%u aggregate=pending stop_reason=already-complete\n",target,group);
        (void)close(lockfd);
        return 0;
    }
    Progress progress;
    double invocation_seconds=0.0;
    unsigned int completed_at_start, group_units_at_start, units_done_this_call=0U;
    Count already_done;
    if (clock_gettime(CLOCK_MONOTONIC,&progress.invocation_start)!=0 ||
        !group_total(states,target,group,&already_done,&completed_at_start,&group_units_at_start)) {
        (void)close(lockfd);
        (void)fprintf(stderr,"target=%u stop_reason=progress-accounting-failed\n",target);
        return 1;
    }
    for (unsigned int unit=0U; unit<segment_count_for_target(target); ++unit) {
        double unit_seconds=0.0;
        if (!unit_in_group(target,unit,group) || states[unit].state==SEGMENT_COMPLETE) continue;
        progress_begin_unit(&progress,target,group,unit,completed_at_start+units_done_this_call+1U,group_units_at_start);
        if (!complete_unit_locked(lockfd,target,unit,source_hash,binary_hash,states,&unit_seconds)) { (void)close(lockfd); (void)fprintf(stderr,"target=%u group=%u unit=%u stop_reason=unit-failed\n",target,group,unit); return 1; }
        ++units_done_this_call;
        invocation_seconds+=unit_seconds;
    }
    current_progress = NULL;
    if (!read_manifest(manifest,target,source_hash,binary_hash,states)) { (void)close(lockfd); (void)fprintf(stderr,"target=%u stop_reason=manifest-conflict\n",target); return 1; }
    Count group_count, partial;
    unsigned int complete_group_units, group_units, complete_units, complete_atoms;
    if (!group_total(states,target,group,&group_count,&complete_group_units,&group_units) || complete_group_units!=group_units || !partial_total(states,target,&partial,&complete_units,&complete_atoms)) { (void)close(lockfd); (void)fprintf(stderr,"target=%u stop_reason=group-accounting-failed\n",target); return 1; }
    (void)printf("target=%u group=%u group_units=%u/%u groups=1/%u units=%u/%u atoms=%u/20 elapsed_s_this_call=%.6f stop_reason=completed\n",target,group,complete_group_units,group_units,groups,complete_units,segment_count_for_target(target),complete_atoms,invocation_seconds);
    print_count("group",group_count);
    print_count("partial",partial);
    if (completed_total(states,target,&partial)) {
        char manifest_hash[65],expected[LINE_CAPACITY],aggregate_hash[65]; size_t expected_length;
        if (!sha256_file(manifest,manifest_hash) || !aggregate_text(expected,&expected_length,target,partial,source_hash,binary_hash,manifest_hash) || !publish_aggregate(aggregate,expected,expected_length) || !sha256_file(aggregate,aggregate_hash)) { (void)close(lockfd); (void)fprintf(stderr,"cannot publish aggregate without clobbering\n"); return 1; }
        print_count("aggregate",partial);
        (void)printf("sha256 source=%s binary=%s manifest=%s output=%s\n",source_hash,binary_hash,manifest_hash,aggregate_hash);
    } else (void)printf("aggregate=pending stop_reason=awaiting-other-groups\n");
    (void)close(lockfd);
    return 0;
}
int main(int argc, char **argv) {
    if (argc==2 && strcmp(argv[1],"selftest")==0) return selftest();
    if (argc==4 && strcmp(argv[1],"run")==0) {
        unsigned int n, group;
        if (parse_unsigned(argv[2],&n) && parse_unsigned(argv[3],&group)) return run_group(n,false,group,argv[0]);
        if (parse_unsigned(argv[2],&n) && strcmp(argv[3],"next")==0) return run_group(n,true,0U,argv[0]);
    }
    (void)fprintf(stderr,"usage: %s selftest | %s run TARGET(4..24) GROUP | %s run TARGET(4..24) next\n",argv[0],argv[0],argv[0]);
    return 2;
}
