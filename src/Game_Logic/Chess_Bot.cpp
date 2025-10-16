// Created By Michael Owen some time around July 3
/*
CURRENT PROBLEMS-
Bot can move King into check and checkmate
activating end game state and detect checkmate does not work correctly
*/
#include "Game_Board.h"
#include <chrono>  
#include <random>
uint64_t random64() {
    static std::mt19937_64 rng(std::random_device{}());
    static std::uniform_int_distribution<uint64_t> dist;
    return dist(rng);
}

move Game_Board::Chess_Bot::calculateMove(Array3D<Game_Piece> board,Team t, std::atomic<bool>& aiCancelled) {
    nodesSearched = 0;
    numFutilityCandidates = 0;
    auto startTime = std::chrono::high_resolution_clock::now();

    stripRenderingData(board);

    
    std::vector<move> manuevers = getAllLegalMoves(board, t, blackKingPos);


    if (manuevers.empty()) {
        throw std::runtime_error("No legal moves available.");
        return move();
    }
    std::sort(manuevers.begin(), manuevers.end(), [&](const move& a, const move& b) {
        return find_board_quality(a) > find_board_quality(b);
        });

        // Clear history heuristic
    std::memset(historyHeuristic, 0, sizeof(historyHeuristic));

    // Clear killer moves
    for (int d = 0; d < MAX_DEPTH; ++d) {
        killerMoves[d][0] = move();
        killerMoves[d][1] = move();
    }


    int bestScore = INT_MIN;
    move bestMove = manuevers[0];
    this->currentHash = Zobrist::computeHash(board);

    this->evalScore = evaluateBoard(board, t); // full eval at root


    // Iterative deepening loop
    for (int depth = 1; depth <= plies; ++depth) {
        std::cout << "[AI] Searching depth: " << depth << "\n";

        int localBestScore = INT_MIN;
        move localBestMove = bestMove;

        for (const move& m : manuevers) {
            if (aiCancelled) {
                std::cout << "AI search cancelled.\n";
                return move();  // return dummy move or throw
            }

            Game_Piece captured = simulateMoveInPlace(board, m);
            this->currentHash = Zobrist::updateHash(this->currentHash, m);

            totalGeneratedMoves = 0;
            totalMoveGenCalls = 0;

            int score = -transpositionTable.alphaBeta(*this, board, depth, startingAlpha, startingBeta, Chess_Logic::getOppositeTeam(t), transpositionTable);
            
            //debugging amount of moves
            //if (totalMoveGenCalls > 0) {
            //    double avgMoves = (double)totalGeneratedMoves / totalMoveGenCalls;
            //    std::cout << "Avg legal moves per node: " << avgMoves << std::endl;
            //}

            undoMove(board, m, captured);
            this->currentHash = Zobrist::revertHash(this->currentHash, m);

            if (score > localBestScore) {
                localBestScore = score;
                localBestMove = m;
            }
        }

        // Update outer best move
        bestMove = localBestMove;
        bestScore = localBestScore;

    }

    // End the timer
    auto endTime = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = endTime - startTime;

    std::cout << "AI move calculated in " << duration.count() << " seconds." << std::endl;
    std::cout << "Nodes Searched " << nodesSearched << "\n";
    std::cout << "Futility Candidates " << numFutilityCandidates << "\n";
    std::cout << "Nodes per second: " << nodesSearched / duration.count() << "\n";

    return bestMove;
}

int Game_Board::Chess_Bot::find_board_quality(move m) {
    int score = 0;

    // MVV-LVA scoring for captures
    if (m.take_piece != None) {
        int victimValue = pieceValues[m.take_piece];
        int attackerValue = pieceValues[m.pieceType];
        score += (victimValue * 10) - attackerValue; // Higher reward for low-value attacker taking high-value victim
    }

    // Encourage development and activity
    score += pieceValues[m.pieceType] / 8;

    // Centralization bonus (encourage moving toward center)
    float centerX = 3.5f;
    float centerZ = 3.5f;
    float distFromCenter = glm::distance(glm::vec2(m.newPosition.x, m.newPosition.z), glm::vec2(centerX, centerZ));
    score += static_cast<int>(8 - distFromCenter);  // Max 8, min ~0

    // Pawn advancement
    if (m.pieceType == Pawn) {
        int direction = (m.team == White) ? 1 : -1;
        int advancement = (m.newPosition.y - m.origionalPosition.y) * direction;
        if (advancement > 0) {
            score += advancement * 3;
        }
    }

    return score;
}

int Game_Board::Chess_Bot::evaluateBoard(Array3D<Game_Piece>& board, Team team) {
    int score = 0;
    for (int x = 0; x < board.getWidth(); ++x) {
        for (int y = 0; y < board.getHeight(); ++y) {
            for (int z = 0; z < board.getDepth(); ++z) {
                score += evaluatePiece(board(x, y, z), x, y, z,board);
            }
        }
    }
    return score;
}

int Game_Board::Chess_Bot::TranspositionTable::alphaBeta(Chess_Bot& bot, Array3D<Game_Piece>& board, int depth, int alpha, int beta, Team team, TranspositionTable& tt) {
    bot.nodesSearched++;
    uint64_t hash = bot.currentHash;
    TTEntry* entry = tt.probe(hash);
    if (entry != nullptr) bot.ttHits++; else bot.ttMisses++;

    int originalAlpha = alpha;

    if (entry != nullptr && entry->depth >= depth) {
        if (entry->flag == EXACT)
            return entry->score;
        else if (entry->flag == LOWERBOUND)
            alpha = std::max(alpha, entry->score);
        else if (entry->flag == UPPERBOUND)
            beta = std::min(beta, entry->score);

        if (alpha >= beta)
            return entry->score;
    }

    if (depth == 0) {
        auto start4 = std::chrono::high_resolution_clock::now();
        int qVal = bot.quiescenceSearch(board, alpha, beta, team, hash, bot.blackKingPos, 0);
        auto end4 = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> duration4 = end4 - start4;
        //std::cout << "[PROFILE] quiescenceSearch took: " << duration4 << "\n";

        return qVal;

    }

    int bestScore = INT_MIN;
    move bestMove;
    glm::vec3 kingPos = (team == Black) ? bot.blackKingPos : bot.whiteKingPos;

    auto moves = bot.getAllLegalMoves(board, team, kingPos);
    bot.totalGeneratedMoves += moves.size();
    bot.totalMoveGenCalls++; 


    if (entry && entry->flag == EXACT) {
        // Move the TT best move to front of list if it's in the current move list
        auto it = std::find(moves.begin(), moves.end(), entry->bestMove);
        if (it != moves.end()) {
            std::iter_swap(moves.begin(), it); // Put TT move first
        }
    }
    //History Heuristics and Killer moves
    // Sort moves by killer + history heuristic
    std::sort(moves.begin(), moves.end(), [&](const move& a, const move& b) {
        bool aIsKiller = (a == bot.killerMoves[depth][0] || a == bot.killerMoves[depth][1]);
        bool bIsKiller = (b == bot.killerMoves[depth][0] || b == bot.killerMoves[depth][1]);



        if (aIsKiller != bIsKiller)
            return aIsKiller; // Killer moves first

        // Captures before non-captures
        if ((a.take_piece != None) != (b.take_piece != None))
            return a.take_piece != None;

        // MVV-LVA (Most Valuable Victim - Least Valuable Attacker)
        if (a.take_piece != None && b.take_piece != None) {
            int aScore = bot.pieceValues[a.take_piece] - bot.pieceValues[a.pieceType];
            int bScore = bot.pieceValues[b.take_piece] - bot.pieceValues[b.pieceType];
            if (aScore != bScore)
                return aScore > bScore;
        }

        // History Heuristic fallback (safe-guarded)
        int aHist = 0, bHist = 0;
        if (a.pieceType >= 0 && a.pieceType < 7 &&
            a.newPosition.x >= 0 && a.newPosition.x < 8 &&
            a.newPosition.y >= 0 && a.newPosition.y < 3 &&
            a.newPosition.z >= 0 && a.newPosition.z < 8)
        {
            aHist = bot.historyHeuristic[a.pieceType]
                [(int)a.newPosition.x]
                [(int)a.newPosition.y]
                [(int)a.newPosition.z];
        }

        if (b.pieceType >= 0 && b.pieceType < 7 &&
            b.newPosition.x >= 0 && b.newPosition.x < 8 &&
            b.newPosition.y >= 0 && b.newPosition.y < 3 &&
            b.newPosition.z >= 0 && b.newPosition.z < 8)
        {
            bHist = bot.historyHeuristic[b.pieceType]
                [(int)b.newPosition.x]
                [(int)b.newPosition.y]
                [(int)b.newPosition.z];
        }



        return aHist > bHist;
        });

    int moveIndex = 0;

    // --- Null Move Pruning ---
    if (depth >= 3 && !Chess_Logic::is_king_in_check(board, team, kingPos)) {
        int R = 2; // Reduction (commonly 2 or 3)

        // Make a "null move" — skip turn
        bot.currentHash = Game_Board::Chess_Bot::Zobrist::nullMoveHash(bot.currentHash, team); // optional
        int nullScore = -alphaBeta(bot, board, depth - 1 - R, -beta, -beta + 1, Chess_Logic::getOppositeTeam(team), tt);
        bot.currentHash = Game_Board::Chess_Bot::Zobrist::revertNullMoveHash(bot.currentHash, team); // optional

        if (nullScore >= beta) {
            return beta; // Fail-hard cutoff
        }
    }


    for (const move& m : moves) {

        int score;

        // === LMR condition ===
        bool isKiller = (m == bot.killerMoves[depth][0] || m == bot.killerMoves[depth][1]);

        bool isCapture = (m.take_piece != None);
        bool reduce = (depth >= 3 && !isCapture && !isKiller && moveIndex >= 3);

        //Late Move Pruning
        if (depth <= 3 && moveIndex > 10 && !isCapture && !isKiller)
            continue; // Don't search unpromising late moves


        Game_Piece captured = bot.simulateMoveInPlace(board, m);
        bot.currentHash = Game_Board::Chess_Bot::Zobrist::updateHash(bot.currentHash, m);



        //Futility Pruning
        if (depth == 1 && !isCapture && !Chess_Logic::is_king_in_check(board, team, kingPos)) {

            int margin = 25 + 3 * depth;

            if (bot.evalScore + margin <= alpha) {
                bot.numFutilityCandidates++;
                bot.undoMove(board, m, captured);
                bot.currentHash = Game_Board::Chess_Bot::Zobrist::revertHash(bot.currentHash, m);
                continue; // Skip searching this move
            }
        }

        if (reduce) {
            score = -alphaBeta(bot, board, depth - 2, -alpha - 1, -alpha, Chess_Logic::getOppositeTeam(team), tt);

            // If it fails high, re-search full depth
            if (score > alpha && score < beta) {
                score = -alphaBeta(bot, board, depth - 1, -beta, -alpha, Chess_Logic::getOppositeTeam(team), tt);
            }
        }
        else {
            score = -alphaBeta(bot, board, depth - 1, -beta, -alpha, Chess_Logic::getOppositeTeam(team), tt);
        }

        bot.undoMove(board, m, captured);
        bot.currentHash = Game_Board::Chess_Bot::Zobrist::revertHash(bot.currentHash, m);

        
        if (bot.currentHash != hash) {
            std::cout << "Hashes are not the same" << std::endl;
        }

        if (score > bestScore) {
            bestScore = score;
            bestMove = m;
        }

        alpha = std::max(alpha, score);

        if (alpha >= beta) {
            if (m.take_piece == None && !isKiller) {
                const auto& km0 = bot.killerMoves[depth][0];
                const auto& km1 = bot.killerMoves[depth][1];

                if (!(km0 == m || km1 == m)) {

                    // Shift current killer[0] to killer[1], and insert new move as killer[0]
                    bot.killerMoves[depth][1] = bot.killerMoves[depth][0];
                    bot.killerMoves[depth][0] = m;
                }
            }


            bot.historyHeuristic[m.pieceType][(int)m.newPosition.x][(int)m.newPosition.y][(int)m.newPosition.z] += depth * depth;


            break; // Beta cutoff
        }

        moveIndex++;
    }

    // Store in transposition table
    TTFlag flag = (bestScore <= originalAlpha) ? UPPERBOUND :
        (bestScore >= beta) ? LOWERBOUND : EXACT;
    tt.store(hash, depth, bestScore, flag, bestMove);

    return bestScore;
}

void Game_Board::Chess_Bot::Zobrist::init(int width, int height, int depth) {
    std::mt19937_64 rng(0xDEADBEEF);
    std::uniform_int_distribution<uint64_t> dist;
    Zobrist::sideToMoveHash = random64(); // Use your random generator


    table.resize(width);
    for (int x = 0; x < width; ++x) {
        table[x].resize(height);
        for (int y = 0; y < height; ++y) {
            table[x][y].resize(depth);
            for (int z = 0; z < depth; ++z) {
                table[x][y][z].resize(NUM_PIECES);
                for (int p = 0; p < NUM_PIECES; ++p) {
                    table[x][y][z][p] = dist(rng);
                }
            }
        }
    }
}

uint64_t Game_Board::Chess_Bot::Zobrist::computeHash(Array3D<Game_Piece>& board) {
    uint64_t h = 0;

    for (int x = 0; x < board.getWidth(); ++x) {
        for (int y = 0; y < board.getHeight(); ++y) {
            for (int z = 0; z < board.getDepth(); ++z) {
                const Game_Piece& p = board(x, y, z);
                int idx = pieceIndex(p.getType(), p.getTeam());
                if (idx >= 0) {
                    // Add bounds checks to be safe
                    if (x < table.size() && y < table[x].size() && z < table[x][y].size() && idx < table[x][y][z].size()) {
                        h ^= table[x][y][z][idx];
                    }
                    else {
                        std::cout << "error: x, " << x << " y: " << y << " z: " << z  << " idx: " << idx << std::endl;
                    }
                }


            }
        }
    }
    return h;
}

uint64_t Game_Board::Chess_Bot::Zobrist::updateHash(uint64_t currentHash, const move& m) {
    int ox = (int)m.origionalPosition.x;
    int oy = (int)m.origionalPosition.y;
    int oz = (int)m.origionalPosition.z;

    int nx = (int)m.newPosition.x;
    int ny = (int)m.newPosition.y;
    int nz = (int)m.newPosition.z;

    int fromIdx = pieceIndex(m.pieceType, m.team);
    int toIdx = fromIdx;


    currentHash ^= table[ox][oy][oz][fromIdx]; // Remove piece from origin

    if (m.take_piece != None) {
        // Opponent captured piece index (flip team)
        Team oppTeam = (m.team == White) ? Black : White;
        int capIdx = pieceIndex(m.take_piece, oppTeam);

        currentHash ^= table[nx][ny][nz][capIdx]; // Remove captured piece
    }

    currentHash ^= table[nx][ny][nz][toIdx]; // Add piece at destination

    return currentHash;
}

uint64_t Game_Board::Chess_Bot::Zobrist::revertHash(uint64_t currentHash, const move& m) {
    int ox = (int)m.origionalPosition.x;
    int oy = (int)m.origionalPosition.y;
    int oz = (int)m.origionalPosition.z;

    int nx = (int)m.newPosition.x;
    int ny = (int)m.newPosition.y;
    int nz = (int)m.newPosition.z;

    int fromIdx = pieceIndex(m.pieceType, m.team);
    int toIdx = fromIdx;

    currentHash ^= table[nx][ny][nz][toIdx]; // Remove piece from destination

    if (m.take_piece != None) {
        Team oppTeam = (m.team == White) ? Black : White;
        int capIdx = pieceIndex(m.take_piece, oppTeam);
        currentHash ^= table[nx][ny][nz][capIdx]; // Add captured piece back
    }

    currentHash ^= table[ox][oy][oz][fromIdx]; // Add piece back at origin

    return currentHash;
}

Game_Piece Game_Board::Chess_Bot::simulateMoveInPlace(Array3D<Game_Piece>& board, const move& m) {
    Game_Piece& from = board(m.origionalPosition.x,m.origionalPosition.y,m.origionalPosition.z);
    Game_Piece& to = board(m.newPosition.x,m.newPosition.y,m.newPosition.z);

    Game_Piece captured = to; // Save for undo

    // Remove evaluation for the moving piece from old position
    evalScore -= evaluatePiece(from, m.origionalPosition.x, m.origionalPosition.y, m.origionalPosition.z,board);

    // Remove evaluation for captured piece (if any)
    if (captured.getTeam() != Null) {
        evalScore -= evaluatePiece(captured, m.newPosition.x, m.newPosition.y, m.newPosition.z,board);
    }

    to = from;
    from = Game_Piece(); // Or empty

    // Add evaluation for piece in new position
    evalScore += evaluatePiece(to, m.newPosition.x, m.newPosition.y, m.newPosition.z, board);

    // If the moving piece is the king, update the stored king position
    if (to.getType() == King) {
        if (m.team == White)
            whiteKingPos = m.newPosition;
        else if (m.team == Black)
            blackKingPos = m.newPosition;
    }

    return captured;

}

void Game_Board::Chess_Bot::undoMove(Array3D<Game_Piece>& board, const move& m, const Game_Piece& captured) {

    auto& from = board(m.origionalPosition.x, m.origionalPosition.y, m.origionalPosition.z);
    auto& to = board(m.newPosition.x, m.newPosition.y, m.newPosition.z);

    // Remove eval for moved piece at new position
    evalScore -= evaluatePiece(to, m.newPosition.x, m.newPosition.y, m.newPosition.z, board);

    // Restore the piece to its original location
    from = to;
    to = captured;

    // Add back eval for moved piece in old position
    evalScore += evaluatePiece(from, m.origionalPosition.x, m.origionalPosition.y, m.origionalPosition.z, board);

    if (captured.getTeam() != Null) {
        evalScore += evaluatePiece(captured, m.newPosition.x, m.newPosition.y, m.newPosition.z, board);
    }

    //restore King Position
    if (m.pieceType == King) {
        if (m.team == White)
            whiteKingPos = m.origionalPosition;
        else if (m.team == Black)
            blackKingPos = m.origionalPosition;
    }
}

int Game_Board::Chess_Bot::quiescenceSearch(Array3D<Game_Piece>& board, int alpha, int beta, Team team, uint64_t hash, glm::vec3& k,int qDepth) {
   if (qDepth > 6)
       return evalScore;

   if (evalScore >= beta) return beta;
   if (alpha < evalScore) alpha = evalScore;



   auto moves = getAllLegalMoves(board, team, k);
   for (const move& m : moves) {
       if (m.take_piece == None) continue;  // Only consider captures
       if (m.take_piece != None && pieceValues[m.take_piece] <= pieceValues[m.pieceType]) continue; // bad capture

       Game_Piece captured = simulateMoveInPlace(board, m);
       uint64_t newHash = Zobrist::updateHash(hash, m);
       int score = -quiescenceSearch(board, -beta, -alpha, Chess_Logic::getOppositeTeam(team), newHash,k,qDepth+1);
       undoMove(board, m, captured);

       if (score >= beta) return beta;
       if (score > alpha) alpha = score;
   }

   return alpha;
}

void Game_Board::Chess_Bot::stripRenderingData(Array3D<Game_Piece>& board) {
    //Strip all Vulkan and rendering logic from Game_Piece
    for (int x = 0; x < board.getWidth(); ++x) {
        for (int y = 0; y < board.getHeight(); ++y) {
            for (int z = 0; z < board.getDepth(); ++z) {
                Game_Piece& p = board(x, y, z);
                p.setTexture(nullptr);
                p.setHitboxModel(ModelLoader()); // or a default-constructed one
                p.setLocalTransform(glm::mat4(1.0f));
                p.setWorldTransform(glm::mat4(1.0f));
                // Optional: clear Model pointer if it's not needed
                p.setModel(nullptr);
            }
        }
    }
}

std::vector<move> Game_Board::Chess_Bot::getAllLegalMoves(Array3D<Game_Piece>& board, Team t, glm::vec3& kingPos) {
    std::vector<move> allMoves;

    for (int x = 0; x < board.getWidth(); ++x) {
        for (int y = 0; y < board.getHeight(); ++y) {
            for (int z = 0; z < board.getDepth(); ++z) {
                Game_Piece& piece = board(x, y, z);
                if (piece.getTeam() != t) continue;

                std::vector<glm::vec3> candidates = Chess_Logic::PossibleMoves(board, x, y, z);

                for (const glm::vec3& target : candidates) {
                    Game_Piece& captured = board(target.x, target.y, target.z);
                    move m(t, piece.getType(), glm::vec3(x, y, z), target, captured.getType());

                    // Simulate move
                    Game_Piece original = board(target.x, target.y, target.z);
                    Game_Piece temp = board(x, y, z);
                    board(target.x, target.y, target.z) = temp;
                    board(x, y, z).selfDestruct();
                    currentHash = Game_Board::Chess_Bot::Zobrist::updateHash(currentHash, m);
                    // Update kingPos if this piece is the king
                    glm::vec3 originalKingPos = kingPos;
                    if (temp.getType() == King) {
                        kingPos = target;
                    }

                    // Only check king safety if the move could impact check status
                    bool skipCheckTest = (temp.getType() != King && captured.getType() == None);
                    bool legal = true;
                    if (!skipCheckTest) {
                        legal = !Chess_Logic::is_king_in_check(board, t, kingPos);
                    }

                    // Undo move
                    board(x, y, z) = temp;
                    board(target.x, target.y, target.z) = original;
                    currentHash = Game_Board::Chess_Bot::Zobrist::revertHash(currentHash, m);
                    kingPos = originalKingPos;

                    if (legal) allMoves.push_back(m);
                }
            }
        }
    }

    return allMoves;
}

int Game_Board::Chess_Bot::evaluatePiece(const Game_Piece& p, int x, int y, int z, Array3D<Game_Piece>& board) {
    if (p.getTeam() == Null) return 0;
    int score = 0;
    int multiplier = (p.getTeam() == Bot_Team) ? 1 : -1;

    Piece_Type type = p.getType();

    score += multiplier * pieceValues[type] * 100;

    // Pawn advancement
    if (type == Pawn) {
        int forwardProgress = (p.getTeam() == White) ? (6 - z) : (z - 1);
        score += multiplier * forwardProgress * 10;
    }

    // Centralization

    float centerX;
    float centerZ;
    int centerY;
    float dist;
    // Centralization Bonus (exclude king)
    if (type != King) {
        centerX = 3.5f;
        centerZ = 3.5f;
        centerY = 1;
        dist = glm::distance(glm::vec2(x, z), glm::vec2(centerX, centerZ));
        score += multiplier * static_cast<int>((4.0f - dist) * 10);
    }

    if (type == King) {
        int adjacentAllies = 0;
        int threats = 0;

        for (int dx = -1; dx <= 1; ++dx) {
            for (int dy = -1; dy <= 1; ++dy) {
                for (int dz = -1; dz <= 1; ++dz) {
                    if (dx == 0 && dy == 0 && dz == 0) continue;
                    int nx = x + dx, ny = y + dy, nz = z + dz;
                    if (nx < 0 || ny < 0 || nz < 0 || nx >= board.getWidth() || ny >= board.getHeight() || nz >= board.getDepth())
                        continue;

                    const Game_Piece& neighbor = board(nx, ny, nz);
                    if (neighbor.getTeam() == p.getTeam()) {
                        adjacentAllies++;
                    }
                    else if (neighbor.getTeam() != Null) {
                        threats++;
                    }
                }
            }
        }

        score += multiplier * adjacentAllies * 10;

        if (adjacentAllies <= 2)
            score -= multiplier * 30;


        // Threat penalty
        score -= multiplier * threats * 8;
    }
    if (type == Castle || type == Bishop || type == Queen) {
        bool isOpen = true;
        for (int zz = 0; zz < board.getDepth(); ++zz) {
            if (zz == z) continue;
            const Game_Piece& filePiece = board(x, y, zz);
            if (filePiece.getTeam() == p.getTeam()) {
                isOpen = false;
                break;
            }
        }
        if (isOpen) score += multiplier * 20;
    }

    // --- Bonus: Knights in center (X-Y-Z plane) ---
    if (type == Horse) {
        if (abs(x - centerX) <= 1 && abs(y - centerY) <= 1 && abs(z - centerZ) <= 1) {
            score += multiplier * 15;
        }
    }

    return score;
}

