#include <iostream>
#include <bitset>
#include <cstdint>
#include <string>
#include <vector>
#include <bits/stdc++.h>


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
     0,  0,  0,  5,  5,  0,  0,  0,
    -5,  0,  0,  0,  0,  0,  0, -5,
    -5,  0,  0,  0,  0,  0,  0, -5,
    -5,  0,  0,  0,  0,  0,  0, -5,
    -5,  0,  0,  0,  0,  0,  0, -5,
    -5,  0,  0,  0,  0,  0,  0, -5,
     5, 10, 10, 10, 10, 10, 10,  5,
     0,  0,  0,  0,  0,  0,  0,  0
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



// LSB and MSB helpers
inline int lsb(u64 bb) { return __builtin_ctzll(bb); }
inline int msb(u64 bb) { return 63 - __builtin_clzll(bb); }

enum Color { WHITE, BLACK };

struct Move {
  int from;
  int to;
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

std::string testFen = "4k3/8/8/8/8/8/PPPPRPPP/RNBQKBNR w KQkq - 0 1";

u64 RookAttackTable[64][4096];   // max 2^12 = 4096
u64 BishopAttackTable[64][512];  // max 2^9  = 512

u64 RookMasks[64];
u64 BishopMasks[64];

int rookBits[64];
int bishopBits[64];

int popcount(u64 x) { return __builtin_popcountll(x); }

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

u64 getPawnAttacks(u64 bit, Color side, Board& board) {
    Board board;
    u64 attacks = 0ULL;
    attacks |= (bit << 8);
    u64 captures = 0ULL;
         
    if (bit & (side == WHITE) & RANK_2) {
        std::cout << "White pawn hasn't moved" << std::endl;
        attacks |= (bit << 16);
        captures |= (bit <<  7), (bit << 9);

    } else if (bit & (side != WHITE) & RANK_7) {
        std::cout << "Black pawn hasn't moved" << std::endl;
        attacks |= (bit >> 16);
        captures |= (bit >> 7), (bit >> 9);
    };

    if (side == WHITE) {
        captures |= (bit << 7), (bit << 9);
    } else {
        captures |= (bit >> 7), (bit >> 9);
    };
    
    if (captures & board.all_black) {
        attacks |= captures;
    } else if (captures & board.all_white) {
        attacks |= captures;
    };
    
    return attacks
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
u64 rookAttacksOnTheFly(int sq, u64 block) {
    u64 attacks = 0ULL;
    int rank = sq / 8, file = sq % 8;
    for (int r = rank + 1; r <= 7; r++) { attacks |= 1ULL << (r * 8 + file); if (block & (1ULL << (r*8+file))) break; }
    for (int r = rank - 1; r >= 0; r--) { attacks |= 1ULL << (r * 8 + file); if (block & (1ULL << (r*8+file))) break; }
    for (int f = file + 1; f <= 7; f++) { attacks |= 1ULL << (rank*8 + f); if (block & (1ULL << (rank*8+f))) break; }
    for (int f = file - 1; f >= 0; f--) { attacks |= 1ULL << (rank*8 + f); if (block & (1ULL << (rank*8+f))) break; }
    return attacks;
}

u64 bishopAttacksOnTheFly(int sq, u64 block) {
    u64 attacks = 0ULL;
    int rank = sq / 8, file = sq % 8;
    for (int r = rank+1, f=file+1; r<=7 && f<=7; r++,f++){ attacks |= 1ULL<<(r*8+f); if(block & (1ULL<<(r*8+f))) break; }
    for (int r = rank+1, f=file-1; r<=7 && f>=0; r++,f--){ attacks |= 1ULL<<(r*8+f); if(block & (1ULL<<(r*8+f))) break; }
    for (int r = rank-1, f=file+1; r>=0 && f<=7; r--,f++){ attacks |= 1ULL<<(r*8+f); if(block & (1ULL<<(r*8+f))) break; }
    for (int r = rank-1, f=file-1; r>=0 && f>=0; r--,f--){ attacks |= 1ULL<<(r*8+f); if(block & (1ULL<<(r*8+f))) break; }
    return attacks;
}

// Init attack tables
void initRookAttacks() { /* fill in RookAttackTable as in your original code */ }
void initBishopAttacks() { /* fill in BishopAttackTable as in your original code */ }

u64 getRookAttacks(int sq, u64 occ) { return rookAttacksOnTheFly(sq, occ); }
u64 getBishopAttacks(int sq, u64 occ) { return bishopAttacksOnTheFly(sq, occ); }
u64 getQueenAttacks(int sq, u64 occ) { return getRookAttacks(sq, occ) | getBishopAttacks(sq, occ); }

// Move / capture helpers
void movePiece(u64 &pieceBB, int fromSq, int toSq) {
    pieceBB &= ~(1ULL << fromSq);
    pieceBB |=  (1ULL << toSq);
}

void capturePiece(u64 &pieceBB, int sq) {
    pieceBB &= ~(1ULL << sq);
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
        if(getBishopAttacks(kingSq, occ) & (board.bishops_black | board.queens_black)) return true;
        if(getRookAttacks(kingSq, occ) & (board.rooks_black | board.queens_black)) return true;
        if(getKingAttacks(1ULL << kingSq) & board.king_black) return true;
    } else {
        u64 pawnAttacks = ((board.pawns_white >> 7) & ~FILE_A) |
                          ((board.pawns_white >> 9) & ~FILE_H);
        if(pawnAttacks & kingBB) return true;
        if(getKnightAttacks(1ULL << kingSq) & board.knights_white) return true;
        if(getBishopAttacks(kingSq, occ) & (board.bishops_white | board.queens_white)) return true;
        if(getRookAttacks(kingSq, occ) & (board.rooks_white | board.queens_white)) return true;
        if(getKingAttacks(1ULL << kingSq) & board.king_white) return true;
    }

    return false;
}





std::vector<Move> generateKnightMoves(Board& board, Color side) {
  std::vector<Move> moves;

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

std::vector<Move> generateKingMoves(const Board& board, Color side) {
  std::vector<Move> moves;

  u64 king = (side == WHITE) ? board.king_white : board.king_black;
  u64 ownPieces = (side == WHITE) ? board.all_white : board.all_black;

  
  int fromSq = lsb(king);
  
  u64 attacks = getKingAttacks(1ULL << fromSq);
  attacks &= ~ownPieces; // can't capture own pieces
  
  while (attacks) {
      int toSq = lsb(attacks);
      attacks &= attacks - 1;
      moves.push_back({fromSq, toSq});
  }
    
    
  return moves;
}



std::vector<Move> generatePawnMoves(Board& board, Color side) {
  std::vector<Move> moves;

  u64 pawns = (side == WHITE) ? board.pawns_white : board.pawns_black;
  u64 ownPieces = (side == WHITE) ? board.all_white : board.all_black;

  while (knights) {
        int fromSq = lsb(pawns);
        pawns &= pawns - 1;
        
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

int main() {
    Board board = fenUnloader(testFen);
    
    

    initMasks();
    // initRookAttacks();
    // initBishopAttacks();

    std::cout << Move << endl;

    int sq = 27; // d4
    
    //print_bitboard(getRookAttacks(sq, occ));
    //print_bitboard(getBishopAttacks(sq, occ));
    //print_bitboard(getQueenAttacks(sq, occ));

    if(isKingInCheck(WHITE, board)) {
        std::cout << "White king is in check!\n";
    };
    if(isKingInCheck(BLACK, board)) {
        std::cout << "Black king is in check!\n";
    };

    return 0;
}