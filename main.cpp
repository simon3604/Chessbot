#include <iostream>
#include <bitset>
#include <cstdint>
#include <string>
#include <vector>
#include <bits/stdc++.h>
#include <cstdlib>
#include <ctime>


using u64 = uint64_t;

constexpr u64 FILE_A  = 0x0101010101010101ULL;
constexpr u64 FILE_B  = (FILE_A << 1);
constexpr u64 FILE_C  = (FILE_A << 2);
constexpr u64 FILE_D  = (FILE_A << 3);
constexpr u64 FILE_E  = (FILE_A << 4);
constexpr u64 FILE_F  = (FILE_A << 5);
constexpr u64 FILE_G  = (FILE_A << 6);
constexpr u64 FILE_H  = (FILE_A << 7);

constexpr u64 RANK_1 = 0x00000000000000FFULL;
constexpr u64 RANK_2 = RANK_1 << 8;
constexpr u64 RANK_3 = RANK_1 << 16;
constexpr u64 RANK_4 = RANK_1 << 24;
constexpr u64 RANK_5 = RANK_1 << 32;
constexpr u64 RANK_6 = RANK_1 << 40;
constexpr u64 RANK_7 = RANK_1 << 48;
constexpr u64 RANK_8 = RANK_1 << 56;


constexpr int PAWN_VALUE   = 100;
constexpr int KNIGHT_VALUE = 320;
constexpr int BISHOP_VALUE = 330;
constexpr int ROOK_VALUE   = 500;
constexpr int QUEEN_VALUE  = 900;
constexpr int KING_VALUE   = 20000;

// Piece-square tables (simplified, for white perspective)
// Indexed by square 0..63, where 0 = a1, 63 = h8
constexpr int PST_PAWN[64] = {
      0, 0, 0, 0, 0, 0, 0, 0,
     50, 50, 50, 50, 50, 50, 50, 50,
     10, 10, 20, 30, 30, 20, 10, 10,
      5,  5, 10, 25, 25, 10,  5,  5,
      0,  0,  0, 20, 20,  0,  0,  0,
      5, -5,-10,  0,  0,-10, -5,  5,
      5, 10, 10,-20,-20, 10, 10,  5,
      0,  0,  0,  0,  0,  0,  0,  0
};

constexpr int PST_KNIGHT[64] = {
    -50,-40,-30,-30,-30,-30,-40,-50,
    -40,-20,  0,  5,  5,  0,-20,-40,
    -30,  5, 10, 15, 15, 10,  5,-30,
    -30,  0, 15, 20, 20, 15,  0,-30,
    -30,  5, 15, 20, 20, 15,  5,-30,
    -30,  0, 10, 15, 15, 10,  0,-30,
    -40,-20,  0,  0,  0,  0,-20,-40,
    -50,-40,-30,-30,-30,-30,-40,-50
};

constexpr int PST_BISHOP[64] = {
    -20,-10,-10,-10,-10,-10,-10,-20,
    -10,  0,  0,  0,  0,  0,  0,-10,
    -10,  0,  5, 10, 10,  5,  0,-10,
    -10,  5,  5, 10, 10,  5,  5,-10,
    -10,  0, 10, 10, 10, 10,  0,-10,
    -10, 10, 10, 10, 10, 10, 10,-10,
    -10,  5,  0,  0,  0,  0,  5,-10,
    -20,-10,-10,-10,-10,-10,-10,-20
};

constexpr int PST_ROOK[64] = {
     0, 5, 10, 15, 15, 10, 5, 0,
    0,10, 20, 25, 25, 20,10, 0,
    0,10, 20, 25, 25, 20,10, 0,
    0,10, 20, 25, 25, 20,10, 0,
    0,10, 20, 25, 25, 20,10, 0,
    0,10, 20, 25, 25, 20,10, 0,
   -5, 0, 10, 15, 15, 10, 0,-5, // rank 7
  -10,-5,  0,  5,  5,  0,-5,-10 
};

constexpr int PST_QUEEN[64] = {
    -20,-10,-10, -5, -5,-10,-10,-20,
    -10,  0,  0,  0,  0,  0,  0,-10,
    -10,  0,  5,  5,  5,  5,  0,-10,
     -5,  0,  5,  5,  5,  5,  0, -5,
      0,  0,  5,  5,  5,  5,  0, -5,
    -10,  5,  5,  5,  5,  5,  0,-10,
    -10,  0,  5,  0,  0,  0,  0,-10,
    -20,-10,-10, -5, -5,-10,-10,-20
};

constexpr int PST_KING[64] = {
    -30,-40,-40,-50,-50,-40,-40,-30,
    -30,-40,-40,-50,-50,-40,-40,-30,
    -30,-40,-40,-50,-50,-40,-40,-30,
    -30,-40,-40,-50,-50,-40,-40,-30,
    -20,-30,-30,-40,-40,-30,-30,-20,
    -10,-20,-20,-20,-20,-20,-20,-10,
     20, 20,  0,  0,  0,  0, 20, 20,
     20, 30, 10,  0,  0, 10, 30, 20
};

constexpr int PST_KING_ENDGAME[64] = {
    -50,-40,-30,-20,-20,-30,-40,-50,
    -30,-20,-10,  0,  0,-10,-20,-30,
    -30,-10, 20, 30, 30, 20,-10,-30,
    -30,-10, 30, 40, 40, 30,-10,-30,
    -30,-10, 30, 40, 40, 30,-10,-30,
    -30,-10, 20, 30, 30, 20,-10,-30,
    -30,-30,  0,  0,  0,  0,-30,-30,
    -50,-30,-30,-30,-30,-30,-30,-50
};

constexpr std::array<uint64_t, 64> RookMagics = {
    0x8a80104000800020ULL, 0x140002000100040ULL, 0x2801880a0017001ULL, 0x100081001000420ULL,
  0x200020010080420ULL, 0x3001c0002010008ULL, 0x8480008002000100ULL, 0x2080088004402900ULL,
  0x800098204000ULL,     0x2024401000200040ULL, 0x100802000801000ULL, 0x120800800801000ULL,
  0x208808088000400ULL,  0x2802200800400ULL,    0x2200800100020080ULL, 0x801000060821100ULL,
  0x80044006422000ULL,   0x100808020004000ULL,  0x12108a0010204200ULL, 0x140848010000802ULL,
  0x481828014002800ULL,  0x8094004002004100ULL, 0x4010040010010802ULL, 0x20008806104ULL,
  0x100400080208000ULL,  0x2040002120081000ULL, 0x21200680100081ULL,   0x20100080080080ULL,
  0x2000a00200410ULL,    0x20080800400ULL,      0x80088400100102ULL,   0x80004600042881ULL,
  0x4040008040800020ULL, 0x440003000200801ULL,  0x4200011004500ULL,    0x188020010100100ULL,
  0x14800401802800ULL,   0x2080040080800200ULL, 0x124080204001001ULL,  0x200046502000484ULL,
  0x480400080088020ULL,  0x1000422010034000ULL, 0x30200100110040ULL,   0x100021010009ULL,
  0x2002080100110004ULL, 0x202008004008002ULL,  0x20020004010100ULL,   0x2048440040820001ULL,
  0x101002200408200ULL,  0x40802000401080ULL,   0x4008142004410100ULL, 0x2060820c0120200ULL,
  0x1001004080100ULL,    0x20c020080040080ULL,  0x2935610830022400ULL, 0x44440041009200ULL,
  0x280001040802101ULL,  0x2100190040002085ULL, 0x80c0084100102001ULL, 0x4024081001000421ULL,
  0x20030a0244872ULL,    0x12001008414402ULL,   0x2006104900a0804ULL,  0x1004081002402ULL
};

constexpr std::array<uint64_t, 64> BishopMagics = {
    0x40040844404084ULL,   0x2004208a004208ULL,   0x10190041080202ULL,   0x108060845042010ULL,
  0x581104180800210ULL,  0x2112080446200010ULL, 0x1080820820060210ULL, 0x3c0808410220200ULL,
  0x4050404440404ULL,    0x21001420088ULL,      0x24d0080801082102ULL, 0x1020a0a020400ULL,
  0x40308200402ULL,      0x4011002100800ULL,    0x401484104104005ULL,  0x801010402020200ULL,
  0x400210c3880100ULL,   0x404022024108200ULL,  0x810018200204102ULL,  0x4002801a02003ULL,
  0x85040820080400ULL,   0x810102c808880400ULL, 0xe900410884800ULL,    0x8002020480840102ULL,
  0x220200865090201ULL,  0x2010100a02021202ULL, 0x152048408022401ULL,  0x20080002081110ULL,
  0x4001001021004000ULL, 0x800040400a011002ULL, 0xe4004081011002ULL,   0x1c004001012080ULL,
  0x8004200962a00220ULL, 0x8422100208500202ULL, 0x2000402200300c08ULL, 0x8646020080080080ULL,
  0x80020a0200100808ULL, 0x2010004880111000ULL, 0x623000a080011400ULL, 0x42008c0340209202ULL,
  0x209188240001000ULL,  0x400408a884001800ULL, 0x110400a6080400ULL,   0x1840060a44020800ULL,
  0x90080104000041ULL,   0x201011000808101ULL,  0x1a2208080504f080ULL, 0x8012020600211212ULL,
  0x500861011240000ULL,  0x180806108200800ULL,  0x4000020e01040044ULL, 0x300000261044000aULL,
  0x802241102020002ULL,  0x20906061210001ULL,   0x5a84841004010310ULL, 0x4010801011c04ULL,
  0xa010109502200ULL,    0x4a02012000ULL,       0x500201010098b028ULL, 0x8040002811040900ULL,
  0x28000010020204ULL,   0x6000020202d0240ULL,  0x8918844842082200ULL, 0x4010011029020020ULL
};

constexpr int RookShifts[64] = {
  52, 53, 53, 53, 53, 53, 53, 52,
  53, 54, 54, 54, 54, 54, 54, 53,
  53, 54, 54, 54, 54, 54, 54, 53,
  53, 54, 54, 54, 54, 54, 54, 53,
  53, 54, 54, 54, 54, 54, 54, 53,
  53, 54, 54, 54, 54, 54, 54, 53,
  53, 54, 54, 54, 54, 54, 54, 53,
  52, 53, 53, 53, 53, 53, 53, 52
};

constexpr int BishopShifts[64] = {
  58, 59, 59, 59, 59, 59, 59, 58,
  59, 59, 59, 59, 59, 59, 59, 59,
  59, 59, 57, 57, 57, 57, 59, 59,
  59, 59, 57, 55, 55, 57, 59, 59,
  59, 59, 57, 55, 55, 57, 59, 59,
  59, 59, 57, 57, 57, 57, 59, 59,
  59, 59, 59, 59, 59, 59, 59, 59,
  58, 59, 59, 59, 59, 59, 59, 58
};


// LSB and MSB helpers
inline int lsb(u64 bb) { return __builtin_ctzll(bb); }
inline int msb(u64 bb) { return 63 - __builtin_clzll(bb); }

enum Color { WHITE, BLACK };

struct Move {
  int from;
  int to;
  int from2;
  int to2;
  u64 capturedPiece;
};

struct Board {
    u64 pawns_white;
    u64 knights_white;
    u64 bishops_white;
    u64 rooks_white;
    u64 queens_white;
    u64 king_white;

    u64 pawns_black;
    u64 knights_black;
    u64 bishops_black;
    u64 rooks_black;
    u64 queens_black;
    u64 king_black;

    // Optional: cached info
    u64 all_white;
    u64 all_black;
};

std::string startFen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

bool canCastleKingside_white = true; 
bool canCastleQueenside_white = true;

bool canCastleKingside_black = true; 
bool canCastleQueenside_black = true;

std::vector<std::vector<u64>> RookAttackTable(64);
std::vector<std::vector<u64>> BishopAttackTable(64);


u64 RookMasks[64];
u64 BishopMasks[64];

int rookBits[64];
int bishopBits[64];

int popcount(u64 x) { return __builtin_popcountll(x); }

int evaluate(const Board& board) {
    int score = 0;

    // Material + PST bonuses
    for (int sq = 0; sq < 64; sq++) {
        u64 mask = 1ULL << sq;

        // White pieces
        if (board.pawns_white & mask)   score += PAWN_VALUE   + PST_PAWN[sq];
        if (board.knights_white & mask) score += KNIGHT_VALUE + PST_KNIGHT[sq];
        if (board.bishops_white & mask) score += BISHOP_VALUE + PST_BISHOP[sq];
        if (board.rooks_white & mask)   score += ROOK_VALUE   + PST_ROOK[sq];
        if (board.queens_white & mask)  score += QUEEN_VALUE  + PST_QUEEN[sq];
        if (board.king_white & mask) {
            // Decide midgame or endgame
            int totalMaterial = popcount(board.pawns_white) * PAWN_VALUE
                              + popcount(board.knights_white) * KNIGHT_VALUE
                              + popcount(board.bishops_white) * BISHOP_VALUE
                              + popcount(board.rooks_white) * ROOK_VALUE
                              + popcount(board.queens_white) * QUEEN_VALUE;
            if (totalMaterial > 1400) score += KING_VALUE + PST_KING[sq]; // midgame
            else score += KING_VALUE + PST_KING_ENDGAME[sq]; // endgame
        }

        // Black pieces (flip PST)
        if (board.pawns_black & mask)   score -= PAWN_VALUE   + PST_PAWN[63 - sq];
        if (board.knights_black & mask) score -= KNIGHT_VALUE + PST_KNIGHT[63 - sq];
        if (board.bishops_black & mask) score -= BISHOP_VALUE + PST_BISHOP[63 - sq];
        if (board.rooks_black & mask)   score -= ROOK_VALUE   + PST_ROOK[63 - sq];
        if (board.queens_black & mask)  score -= QUEEN_VALUE  + PST_QUEEN[63 - sq];
        if (board.king_black & mask) {
            int totalMaterial = popcount(board.pawns_black) * PAWN_VALUE
                              + popcount(board.knights_black) * KNIGHT_VALUE
                              + popcount(board.bishops_black) * BISHOP_VALUE
                              + popcount(board.rooks_black) * ROOK_VALUE
                              + popcount(board.queens_black) * QUEEN_VALUE;
            if (totalMaterial > 1400) score -= KING_VALUE + PST_KING[63 - sq]; // midgame
            else score -= KING_VALUE + PST_KING_ENDGAME[63 - sq]; // endgame
        }
    }

    return score;
}


void print_bitboard(u64 bb) {
    for (int rank = 7; rank >= 0; rank--) {
        for (int file = 0; file < 8; file++) {
            int square = rank * 8 + file;
            std::cout << ((bb >> square) & 1ULL) << " ";
        }
        std::cout << "\n";
    }
    std::cout << "\n";
}

// FEN -> Board
Board fenUnloader(const std::string& fen) {
    int square = 56;
    Board board = {};
    for (char c : fen) {
        if (c == ' ') break;
        else if (c == '/') { square -= 16; }
        else if (isdigit(c)) { square += c - '0'; }
        else {
            u64 bit = 1ULL << square;
            switch(c) {
                case 'P': 
                    board.pawns_white   |= bit; 
                    board.all_white |= bit;
                    break;
                case 'N': 
                    board.knights_white |= bit; 
                    board.all_white |= bit;
                    break;
                case 'B': 
                    board.bishops_white |= bit; 
                    board.all_white |= bit;
                    break;
                case 'R': 
                    board.rooks_white   |= bit; 
                    board.all_white |= bit;
                    break;
                case 'Q': 
                    board.queens_white  |= bit; 
                    board.all_white |= bit;
                    break;
                case 'K': 
                    board.king_white    |= bit; 
                    board.all_white |= bit;
                    break;
                case 'p': 
                    board.pawns_black   |= bit; 
                    board.all_black |= bit;
                    break;
                case 'n': 
                    board.knights_black |= bit; 
                    board.all_black |= bit;
                    break;
                case 'b': 
                    board.bishops_black |= bit; 
                    board.all_black |= bit;
                    break;
                case 'r': 
                    board.rooks_black   |= bit; 
                    board.all_black |= bit;
                    break;
                case 'q': 
                    board.queens_black  |= bit; 
                    board.all_black |= bit;
                    break;
                case 'k': 
                    board.king_black    |= bit; 
                    board.all_black |= bit;
                    break;
            }
            square++;
        }
    }
    return board;
}

u64* getTypeBB(int pos, Board& board) {
    u64 mask = 1ULL << pos;
    if (mask & board.pawns_white)  return &board.pawns_white;
    if (mask & board.rooks_white)  return &board.rooks_white;
    if (mask & board.knights_white) return &board.knights_white;
    if (mask & board.bishops_white) return &board.bishops_white;
    if (mask & board.queens_white)  return &board.queens_white;
    if (mask & board.king_white)   return &board.king_white;

    if (mask & board.pawns_black)  return &board.pawns_black;
    if (mask & board.rooks_black)  return &board.rooks_black;
    if (mask & board.knights_black) return &board.knights_black;
    if (mask & board.bishops_black) return &board.bishops_black;
    if (mask & board.queens_black)  return &board.queens_black;
    if (mask & board.king_black)   return &board.king_black;

    return nullptr;
}

// Rook attacks on the fly
u64 getRookAttacks(int sq, u64 occ) {
    u64 attacks = 0ULL;
    int rank = sq / 8, file = sq % 8;
    for (int r = rank + 1; r <= 7; r++) { 
        attacks |= 1ULL << (r * 8 + file); 
        if (occ & (1ULL << (r*8+file))) break; 
    }
    for (int r = rank - 1; r >= 0; r--) { 
        attacks |= 1ULL << (r * 8 + file); 
        if (occ & (1ULL << (r*8+file))) break; 
    }
    for (int f = file + 1; f <= 7; f++) { 
        attacks |= 1ULL << (rank*8 + f); 
        if (occ & (1ULL << (rank*8+f))) break; 
    }
    for (int f = file - 1; f >= 0; f--) { 
        attacks |= 1ULL << (rank*8 + f); 
        if (occ & (1ULL << (rank*8+f))) break; 
    }
    return attacks;
}

// Bishop attacks on the fly
u64 getBishopAttacks(int sq, u64 occ) {
    u64 attacks = 0ULL;
    int rank = sq / 8, file = sq % 8;

    for (int r = rank+1, f=file+1; r<=7 && f<=7; r++,f++){ 
        attacks |= 1ULL<<(r*8+f); 
        if(occ & (1ULL<<(r*8+f))) break; 
    }
    for (int r = rank+1, f=file-1; r<=7 && f>=0; r++,f--){ 
        attacks |= 1ULL<<(r*8+f); 
        if(occ & (1ULL<<(r*8+f))) break; 
    }
    for (int r = rank-1, f=file+1; r>=0 && f<=7; r--,f++){ 
        attacks |= 1ULL<<(r*8+f); 
        if(occ & (1ULL<<(r*8+f))) break; 
    }
    for (int r = rank-1, f=file-1; r>=0 && f>=0; r--,f--){ 
        attacks |= 1ULL<<(r*8+f); 
        if(occ & (1ULL<<(r*8+f))) break; 
    }
    return attacks;
}

// Queen attacks on the fly (optional)
u64 getQueenAttacks(int sq, u64 occ) { 
    return getRookAttacks(sq, occ) | getBishopAttacks(sq, occ); 
}


// Piece attacks
u64 getKingAttacks(u64 bit) {
    u64 attacks = 0ULL;
    attacks |= (bit << 8);  
    attacks |= (bit >> 8);  
    attacks |= (bit << 1) & ~FILE_H;
    attacks |= (bit >> 1) & ~FILE_A;
    attacks |= (bit << 9) & ~FILE_A;
    attacks |= (bit << 7) & ~FILE_H;
    attacks |= (bit >> 7) & ~FILE_A;
    attacks |= (bit >> 9) & ~FILE_H;
    return attacks;
}

u64 getKnightAttacks(u64 bit) {
    u64 attacks = 0ULL;
    attacks |= (bit << 17) & ~FILE_A;         
    attacks |= (bit << 15) & ~FILE_H;         
    attacks |= (bit << 10) & ~(FILE_A | FILE_B);
    attacks |= (bit << 6)  & ~(FILE_H | FILE_G);
    attacks |= (bit >> 17) & ~FILE_H;         
    attacks |= (bit >> 15) & ~FILE_A;         
    attacks |= (bit >> 10) & ~(FILE_H | FILE_G);
    attacks |= (bit >> 6)  & ~(FILE_A | FILE_B);
    return attacks;
}

u64 getPawnAttacks(u64 bit, Color side, const Board& board) {
    u64 occ = board.all_white | board.all_black;
    u64 moves = 0ULL;

    if (side == WHITE) {
        if (!(bit & RANK_8)) {  // can't move if already on last rank
            u64 oneAhead = bit << 8;
            u64 twoAhead = bit << 16;
            if (!(oneAhead & occ)) {
                moves |= oneAhead;
                if ((bit & RANK_2) && !(twoAhead & occ)) {
                    moves |= twoAhead;
                }
            }
        }
    

        // correct capture directions
        u64 captures = 0ULL;
        captures |= (bit << 9) & ~FILE_A & board.all_black; // capture right
        captures |= (bit << 7) & ~FILE_H & board.all_black; // capture left
        moves |= captures;

    } else {
        if (!(bit & RANK_1)) {
            u64 oneAhead = bit >> 8;
            u64 twoAhead = bit >> 16;
            if (!(oneAhead & occ)) {
                moves |= oneAhead;
                if ((bit & RANK_7) && !(twoAhead & occ))
                    moves |= twoAhead;
            }
        }

        u64 captures = 0ULL;
        captures |= (bit >> 9) & ~FILE_H & board.all_white; // capture right
        captures |= (bit >> 7) & ~FILE_A & board.all_white; // capture left
        moves |= captures;
    }

    
    return moves;
}

// Masks
u64 maskRook(int sq) {
    u64 mask = 0ULL;
    int rank = sq / 8;
    int file = sq % 8;
    for (int f = file + 1; f <= 6; ++f) mask |= 1ULL << (rank * 8 + f);
    for (int f = file - 1; f >= 1; --f) mask |= 1ULL << (rank * 8 + f);
    for (int r = rank + 1; r <= 6; ++r) mask |= 1ULL << (r * 8 + file);
    for (int r = rank - 1; r >= 1; --r) mask |= 1ULL << (r * 8 + file);
    return mask;
}

u64 maskBishop(int sq) {
    u64 mask = 0ULL;
    int rank = sq / 8;
    int file = sq % 8;
    for (int r = rank + 1, f = file + 1; r <= 6 && f <= 6; r++, f++) mask |= 1ULL << (r * 8 + f);
    for (int r = rank + 1, f = file - 1; r <= 6 && f >= 1; r++, f--) mask |= 1ULL << (r * 8 + f);
    for (int r = rank - 1, f = file + 1; r >= 1 && f <= 6; r--, f++) mask |= 1ULL << (r * 8 + f);
    for (int r = rank - 1, f = file - 1; r >= 1 && f >= 1; r--, f--) mask |= 1ULL << (r * 8 + f);
    return mask;
}

void initMasks() {
    for (int sq = 0; sq < 64; ++sq) {
        RookMasks[sq] = maskRook(sq);
        BishopMasks[sq] = maskBishop(sq);
        rookBits[sq] = popcount(RookMasks[sq]);
        bishopBits[sq] = popcount(BishopMasks[sq]);
    }
}

u64 setOccupancy(int index, int bits, u64 mask) {
    u64 occupancy = 0ULL;
    for (int i = 0; i < bits; i++) {
        int square = __builtin_ctzll(mask);
        mask &= mask - 1;
        if (index & (1 << i)) occupancy |= (1ULL << square);
    }
    return occupancy;
}

// Rook/Bishop attacks on the fly
u64 getRookAttackMagics(int sq, u64 occ) {
    occ &= RookMasks[sq];
    int index = (occ * RookMagics[sq]) >> (64 - rookBits[sq]); // safe index
    return RookAttackTable[sq][index];
}

u64 getBishopAttackMagics(int sq, u64 occ) {
    occ &= BishopMasks[sq];
    int index = (occ * BishopMagics[sq]) >> (64 - bishopBits[sq]); // safe index
    return BishopAttackTable[sq][index];
}


void initRookAttacks() {
    for (int sq = 0; sq < 64; ++sq) {
        int bits = rookBits[sq];
        int occupancyCount = 1 << bits;
        RookAttackTable[sq].resize(occupancyCount);

        for (int index = 0; index < occupancyCount; ++index) {
            u64 occ = setOccupancy(index, bits, RookMasks[sq]);
            // The index must match the lookup formula used later
            int magicIndex = (occ * RookMagics[sq]) >> (64 - rookBits[sq]);
            RookAttackTable[sq][magicIndex] = getRookAttacks(sq, occ);
        }
    }
}

void initBishopAttacks() {
    for (int sq = 0; sq < 64; ++sq) {
        int bits = bishopBits[sq];
        int occupancyCount = 1 << bits;
        BishopAttackTable[sq].resize(occupancyCount);

        for (int index = 0; index < occupancyCount; ++index) {
            u64 occ = setOccupancy(index, bits, BishopMasks[sq]);
            // The index must match the lookup formula used later
            int magicIndex = (occ * BishopMagics[sq]) >> (64 - bishopBits[sq]);
            BishopAttackTable[sq][magicIndex] = getBishopAttacks(sq, occ);
        }
    }
}

void undoMove(u64 &pieceBB, int fromSq, int toSq, Color side, Board &board, u64 capturedPiece) {
    // Move piece back
    pieceBB &= ~(1ULL << toSq);
    pieceBB |=  (1ULL << fromSq);
   
    // Restore captured piece
    if (capturedPiece) {
        u64* captureBB = getTypeBB(lsb(capturedPiece), board);
        if (captureBB) *captureBB |= capturedPiece;
    }

    // Update all_white / all_black
    board.all_white = board.pawns_white | board.knights_white | board.bishops_white |
                      board.rooks_white | board.queens_white | board.king_white;
    board.all_black = board.pawns_black | board.knights_black | board.bishops_black |
                      board.rooks_black | board.queens_black | board.king_black;
}

u64 getQueenAttackMagics(int sq, u64 occ) { return getRookAttackMagics(sq, occ) | getBishopAttackMagics(sq, occ); }


u64 movePiece(u64 &pieceBB, int fromSq, int toSq, Color side, Board &board) {
    u64 captured = 0ULL;

    if (side == WHITE) {
        if ((1ULL << toSq) & board.all_black) {
            u64* captureBB = getTypeBB(toSq, board);
            captured = *captureBB & (1ULL << toSq);
            (*captureBB) &= ~(1ULL << toSq);
        }
    } else {
        if ((1ULL << toSq) & board.all_white) {
            u64* captureBB = getTypeBB(toSq, board);
            captured = *captureBB & (1ULL << toSq);
            (*captureBB) &= ~(1ULL << toSq);
        }
    }

    pieceBB &= ~(1ULL << fromSq);
    pieceBB |=  (1ULL << toSq);

    board.all_white = board.pawns_white | board.knights_white | board.bishops_white |
                      board.rooks_white | board.queens_white | board.king_white;
    board.all_black = board.pawns_black | board.knights_black | board.bishops_black |
                      board.rooks_black | board.queens_black | board.king_black;
    
    return captured; // return captured piece for undoing
}


// Check for king in check
bool isKingInCheck(Color side, const Board& board) {
    u64 occ = board.pawns_white | board.knights_white | board.bishops_white |
              board.rooks_white | board.queens_white | board.king_white |
              board.pawns_black | board.knights_black | board.bishops_black |
              board.rooks_black | board.queens_black | board.king_black;

    u64 kingBB = (side == WHITE) ? board.king_white : board.king_black;
    if (!kingBB) return false;
    int kingSq = lsb(kingBB);

    if(side == WHITE){
        u64 pawnAttacks = ((board.pawns_black << 7) & ~FILE_H) |
                          ((board.pawns_black << 9) & ~FILE_A);
        if(pawnAttacks & kingBB) return true;
        if(getKnightAttacks(1ULL << kingSq) & board.knights_black) return true;
        if(getBishopAttackMagics(kingSq, occ) & (board.bishops_black | board.queens_black)) return true;
        if(getRookAttackMagics(kingSq, occ) & (board.rooks_black | board.queens_black)) return true;
        if(getKingAttacks(1ULL << kingSq) & board.king_black) return true;
    } else {
        u64 pawnAttacks = ((board.pawns_white >> 7) & ~FILE_A) |
                          ((board.pawns_white >> 9) & ~FILE_H);
        if(pawnAttacks & kingBB) return true;
        if(getKnightAttacks(1ULL << kingSq) & board.knights_white) return true;
        if(getBishopAttackMagics(kingSq, occ) & (board.bishops_white | board.queens_white)) return true;
        if(getRookAttackMagics(kingSq, occ) & (board.rooks_white | board.queens_white)) return true;
        if(getKingAttacks(1ULL << kingSq) & board.king_white) return true;
    }

    return false;
}


std::vector<Move> generateKnightMoves(Board& board, Color side, std::vector<Move>& moves) {

  u64 knights = (side == WHITE) ? board.knights_white : board.knights_black;
  u64 ownPieces = (side == WHITE) ? board.all_white : board.all_black;

  while (knights) {
        int fromSq = lsb(knights);
        knights &= knights - 1;
        
        u64 attacks = getKnightAttacks(1ULL << fromSq);
        attacks &= ~ownPieces; // can't capture own pieces
        
        while (attacks) {
            int toSq = lsb(attacks);
            attacks &= attacks - 1;
            moves.push_back({fromSq, toSq});
        }
    }
    
    return moves;
}

std::vector<Move> generateKingMoves(Board& board, Color side, std::vector<Move>& moves) {
    u64 king = (side == WHITE) ? board.king_white : board.king_black;
    u64 ownPieces = (side == WHITE) ? board.all_white : board.all_black;
    u64 allPieces = board.all_white | board.all_black;

    int fromSq = lsb(king);
    u64 attacks = getKingAttacks(1ULL << fromSq);
    attacks &= ~ownPieces;

    // Normal king moves
    while (attacks) {
        int toSq = lsb(attacks);
        attacks &= attacks - 1;
        moves.push_back({fromSq, toSq, -1, -1, 0});
    }

    // --- Castling ---
    if (side == WHITE) {
        // White kingside (E1 -> G1, rook H1 -> F1)
        if (canCastleKingside_white &&
            !(allPieces & ((1ULL << 5) | (1ULL << 6))) &&   // F1, G1 empty
            !isKingInCheck(WHITE, board)) {
            
            // Temporarily move king through F1 and G1 to check safety
            Board temp = board;
            movePiece(temp.king_white, 4, 5, WHITE, temp);
            if (!isKingInCheck(WHITE, temp)) {
                movePiece(temp.king_white, 5, 6, WHITE, temp);
                if (!isKingInCheck(WHITE, temp))
                    moves.push_back({4, 6, 7, 5, 0}); // from2=7 (rook), to2=5 (rook move)
            }
        }

        // White queenside (E1 -> C1, rook A1 -> D1)
        if (canCastleQueenside_white &&
            !(allPieces & ((1ULL << 1) | (1ULL << 2) | (1ULL << 3))) && // B1, C1, D1 empty
            !isKingInCheck(WHITE, board)) {
            
            Board temp = board;
            movePiece(temp.king_white, 4, 3, WHITE, temp);
            if (!isKingInCheck(WHITE, temp)) {
                movePiece(temp.king_white, 3, 2, WHITE, temp);
                if (!isKingInCheck(WHITE, temp))
                    moves.push_back({4, 2, 0, 3, 0}); // from2=0 (rook), to2=3 (rook move)
            }
        }
    } else {
        // Black kingside (E8 -> G8, rook H8 -> F8)
        if (canCastleKingside_black &&
            !(allPieces & ((1ULL << 61) | (1ULL << 62))) &&  // F8, G8 empty
            !isKingInCheck(BLACK, board)) {
            
            Board temp = board;
            movePiece(temp.king_black, 60, 61, BLACK, temp);
            if (!isKingInCheck(BLACK, temp)) {
                movePiece(temp.king_black, 61, 62, BLACK, temp);
                if (!isKingInCheck(BLACK, temp))
                    moves.push_back({60, 62, 63, 61, 0});
            }
        }

        // Black queenside (E8 -> C8, rook A8 -> D8)
        if (canCastleQueenside_black &&
            !(allPieces & ((1ULL << 57) | (1ULL << 58) | (1ULL << 59))) &&
            !isKingInCheck(BLACK, board)) {
            
            Board temp = board;
            movePiece(temp.king_black, 60, 59, BLACK, temp);
            if (!isKingInCheck(BLACK, temp)) {
                movePiece(temp.king_black, 59, 58, BLACK, temp);
                if (!isKingInCheck(BLACK, temp))
                    moves.push_back({60, 58, 56, 59, 0});
            }
        }
    }

    return moves;
}


std::vector<Move> generatePawnMoves(Board& board, Color side, std::vector<Move>& moves) {

  u64 pawns = (side == WHITE) ? board.pawns_white : board.pawns_black;
  u64 ownPieces = (side == WHITE) ? board.all_white : board.all_black;

  while (pawns) {
        int fromSq = lsb(pawns);
        pawns &= pawns - 1;
        
        u64 attacks = getPawnAttacks((1ULL << fromSq), side, board);
        attacks &= ~ownPieces; // can't capture own pieces
        
        while (attacks) {
            int toSq = lsb(attacks);
            attacks &= attacks - 1;
            moves.push_back({fromSq, toSq});
        }
    }
    
    return moves;
}

std::vector<Move> generateRookMoves(Board& board, Color side, std::vector<Move>& moves) {
    u64 rooks = (side == WHITE) ? board.rooks_white : board.rooks_black;
    u64 ownPieces = (side == WHITE) ? board.all_white : board.all_black;

    while (rooks) {
        u64 occ = board.all_white | board.all_black;
        int fromSq = lsb(rooks);
        rooks &= rooks - 1;

        u64 attacks = getRookAttackMagics(fromSq, occ);
        attacks &= ~ownPieces;

        while (attacks) {
            int toSq = lsb(attacks);
            attacks &= attacks - 1;
            moves.push_back({fromSq, toSq});
        }
    }

    return moves;
}

std::vector<Move> generateBishopMoves(Board& board, Color side, std::vector<Move>& moves) {
    u64 bishops = (side == WHITE) ? board.bishops_white : board.bishops_black;
    u64 ownPieces = (side == WHITE) ? board.all_white : board.all_black;

    while (bishops) {
        u64 occ = board.all_white | board.all_black;
        int fromSq = lsb(bishops);
        bishops &= bishops - 1;

        u64 attacks = getBishopAttackMagics(fromSq, occ);
        attacks &= ~ownPieces;

        while (attacks) {
            int toSq = lsb(attacks);
            attacks &= attacks - 1;
            moves.push_back({fromSq, toSq});
        }
    }

    return moves;
}

std::vector<Move> generateQueenMoves(Board& board, Color side, std::vector<Move>& moves) {
    u64 queens = (side == WHITE) ? board.queens_white : board.queens_black;
    u64 ownPieces = (side == WHITE) ? board.all_white : board.all_black;

    while (queens) {
        u64 occ = board.all_white | board.all_black;
        int fromSq = lsb(queens);
        queens &= queens - 1;

        u64 attacks = getQueenAttackMagics(fromSq, occ);
        attacks &= ~ownPieces;

        while (attacks) {
            int toSq = lsb(attacks);
            attacks &= attacks - 1;
            moves.push_back({fromSq, toSq});
        }
    }

    return moves;
}


std::string randomMove(std::vector<Move>& moves, Color side, Board& board) {
    
    if (moves.empty()) return "no moves";

    int length = moves.size();

    int idx = rand() % length;
    Move m = moves[idx];

    std::string selectedMove = "from " + std::to_string(m.from) + " to " + std::to_string(m.to);


    u64* pieceBB = getTypeBB(m.from, board);
    if (!pieceBB) {
        return selectedMove + " (no piece at from)";
    }

    movePiece(*pieceBB, m.from, m.to, side, board); 

    board.all_white = 
        board.pawns_white | board.knights_white | board.bishops_white |
        board.rooks_white | board.queens_white | board.king_white;
    board.all_black = 
        board.pawns_black | board.knights_black | board.bishops_black |
        board.rooks_black | board.queens_black | board.king_black;

    return selectedMove;
} 

void printBoardAsLetters(const Board& board) {
    for (int rank = 7; rank >= 0; --rank) { // print from rank 8 to 1
        for (int file = 0; file < 8; ++file) {
            int sq = rank * 8 + file;
            char c = '.';

            u64 mask = 1ULL << sq;

            if (mask & board.pawns_white)   c = 'P';
            else if (mask & board.knights_white) c = 'N';
            else if (mask & board.bishops_white) c = 'B';
            else if (mask & board.rooks_white)   c = 'R';
            else if (mask & board.queens_white)  c = 'Q';
            else if (mask & board.king_white)    c = 'K';
            else if (mask & board.pawns_black)   c = 'p';
            else if (mask & board.knights_black) c = 'n';
            else if (mask & board.bishops_black) c = 'b';
            else if (mask & board.rooks_black)   c = 'r';
            else if (mask & board.queens_black)  c = 'q';
            else if (mask & board.king_black)    c = 'k';

            std::cout << c << " ";
        }
        std::cout << "\n";
    }
}

std::string evalMove(std::vector<Move>& moves, Color side, Board& board) {
    if (moves.empty()) return "no moves";
    Board boardCopy = board;

    int length = moves.size();
    int bestMoveEval = 0;
    int from;
    int to;
    std::cout << bestMoveEval << std::endl;
    static int lastFrom = -1, lastTo = -1;


    //start
    for (int i = 0; i < length; i++) {
        Move m = moves[i];
        u64* pieceBB = getTypeBB(m.from, boardCopy);
        
        u64 capture = 0ULL;

        movePiece(*pieceBB, m.from, m.to, side, boardCopy);
        int eval = evaluate(boardCopy);
        
        if (m.from == lastTo && m.to == lastFrom) {
            eval -= 20; // discourage reversals
        }
        
        std::cout << eval << ", ";
        if ((!bestMoveEval) || (eval > bestMoveEval)) {
            bestMoveEval = eval;
            from = m.from;
            to = m.to;
        } 
        std::cout << bestMoveEval << std::endl;
        undoMove(*pieceBB, m.from, m.to, side, boardCopy, capture);
        lastFrom = from;
        lastTo = to;

    }
    //end

    std::string selectedMove = "from " + std::to_string(from) + " to " + std::to_string(to);


    u64* pieceBB = getTypeBB(from, board);
    if (!pieceBB) {
        return selectedMove + " (no piece at from)";
    }

    movePiece(*pieceBB, from, to, side, board); 

    board.all_white = 
        board.pawns_white | board.knights_white | board.bishops_white |
        board.rooks_white | board.queens_white | board.king_white;
    board.all_black = 
        board.pawns_black | board.knights_black | board.bishops_black |
        board.rooks_black | board.queens_black | board.king_black;

    return selectedMove;
}

int alphaBeta(Board& board, int depth, int alpha, int beta, Color side) {
    if (depth == 0)
        return evaluate(board);

    std::vector<Move> moves;
    generatePawnMoves(board, side, moves);
    generateRookMoves(board, side, moves);
    generateKnightMoves(board, side, moves);
    generateBishopMoves(board, side, moves);
    generateQueenMoves(board, side, moves);
    generateKingMoves(board, side, moves);

    if (side == WHITE) { // maximize
        int value = -1000000;
        for (auto& m : moves) {
            Board copy = board;
            movePiece(*getTypeBB(m.from, copy), m.from, m.to, side, copy);

            int score = alphaBeta(copy, depth - 1, alpha, beta, BLACK);
            value = std::max(value, score);
            alpha = std::max(alpha, score);

            // pruning
            if (alpha >= beta)
                break;
        }
        return value;
    } else { // minimize
        int value = 1000000;
        for (auto& m : moves) {
            Board copy = board;
            movePiece(*getTypeBB(m.from, copy), m.from, m.to, side, copy);

            int score = alphaBeta(copy, depth - 1, alpha, beta, WHITE);
            value = std::min(value, score);
            beta = std::min(beta, score);

            // pruning
            if (beta <= alpha)
                break;
        }
        return value;
    }
}

Move findBestMove(Board& board, Color side, int depth) {
    std::vector<Move> moves;
    generatePawnMoves(board, side, moves);
    generateRookMoves(board, side, moves);
    generateKnightMoves(board, side, moves);
    generateBishopMoves(board, side, moves);
    generateQueenMoves(board, side, moves);
    generateKingMoves(board, side, moves);

    Move bestMove;
    int bestScore = (side == WHITE) ? -1000000 : 1000000;

    for (auto& m : moves) {
        Board copy = board;
        movePiece(*getTypeBB(m.from, copy), m.from, m.to, side, copy);

        int score = alphaBeta(copy, depth - 1, -1000000, 1000000,
                              (side == WHITE) ? BLACK : WHITE);

        if ((side == WHITE && score > bestScore) ||
            (side == BLACK && score < bestScore)) {
            bestScore = score;
            bestMove = m;
        }
    }
    movePiece(*getTypeBB(bestMove.from, board), bestMove.from, bestMove.to, side, board);

    // Handle rook part of castling
    if (bestMove.from2 != -1 && bestMove.to2 != -1) {
        u64* rookBB = getTypeBB(bestMove.from2, board);
        if (rookBB) movePiece(*rookBB, bestMove.from2, bestMove.to2, side, board);
    }

    // Update castle rights if king or rook moved
    if (side == WHITE) {
        if (bestMove.from == 4) canCastleKingside_white = canCastleQueenside_white = false;
        if (bestMove.from == 7 || bestMove.from2 == 7) canCastleKingside_white = false;
        if (bestMove.from == 0 || bestMove.from2 == 0) canCastleQueenside_white = false;
    } else {
        if (bestMove.from == 60) canCastleKingside_black = canCastleQueenside_black = false;
        if (bestMove.from == 63 || bestMove.from2 == 63) canCastleKingside_black = false;
        if (bestMove.from == 56 || bestMove.from2 == 56) canCastleQueenside_black = false;
    }


    return bestMove;
}


int main() {
    initMasks();
    
    for (int sq = 0; sq < 64; sq++) {
        RookAttackTable[sq].resize(1ULL << rookBits[sq]);
        BishopAttackTable[sq].resize(1ULL << bishopBits[sq]);
    }
    
    std::string testFen = "pppppppp/pppppppp/pppppppp/pppppppp/PPPPPPPP/PPPPPPPP/PPPPPPPP/PPPPPPPP w KQkq - 0 1";
    Board board = fenUnloader(startFen);
    std::vector<Move> moves;
    srand(time(0)); // seed random
        
    Color side = WHITE;
   
    initRookAttacks();
    initBishopAttacks();

    for (int t = 0; t < 500; t++) {
        
        generatePawnMoves(board, side, moves);
        generateKnightMoves(board, side, moves);
        generateKingMoves(board, side, moves);
        generateRookMoves(board, side, moves);
        generateBishopMoves(board, side, moves);
        generateQueenMoves(board, side, moves);


        Move best = findBestMove(board, side, 3);
        
        std::cout << "Best move: " << best.from << " → " << best.to << "\n";

    /* std::string moveStr = evalMove(moves, side, board);
        std::cout << "Evaluted move: " << moveStr << "\n";
        print_bitboard(board.all_white);
        */
        moves.clear();
        int eval = evaluate(board);
        std::cout << "Evalution: " << eval << "\n";

        printBoardAsLetters(board);

        int sq = 27; // d4
        u64 occ = 0ULL;



        if (isKingInCheck(WHITE, board)) {
            std::cout << "White king is in check!\n";
        };

        if (isKingInCheck(BLACK, board)) {
            std::cout << "Black king is in check!\n";
        };
        
        std::string w;
        std::cin >> w;
        if (side == WHITE) {
            side = BLACK;
        } else {
            side = WHITE;
        }
        
    }
    return 0;
}