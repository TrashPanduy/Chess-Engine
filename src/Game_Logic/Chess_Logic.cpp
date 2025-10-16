#include "Chess_Logic.h"
std::vector<glm::vec3> Chess_Logic::PossibleMoves(Array3D<Game_Piece>& board,int x, int y, int z) {
    std::vector<glm::vec3> Moves;
    Piece_Type type = board(x,y,z).getType();
    Team team = board(x,y,z).getTeam();

    auto isInside = [&](int xi, int yi, int zi) {
        return xi >= 0 && xi < 8 &&
            yi >= 0 && yi < 3 &&
            zi >= 0 && zi < 8;
        };

    auto isEnemy = [&](int xi, int yi, int zi) {
        return isInside(xi, yi, zi) &&
            board(xi,yi,zi).getTeam() != Null &&
            board(xi,yi,zi).getTeam() != team;
        };

    auto isEmpty = [&](int xi, int yi, int zi) {
        return isInside(xi, yi, zi) &&
            board(xi,yi,zi).getTeam() == Null;
        };

    switch (type) {
    case Pawn: {
        int dir = (team == White) ? -1 : 1;

        // Capture diagonally left
        if (isInside(x - 1, y, z + dir) && isEnemy(x - 1, y, z + dir)) {
            Moves.emplace_back(x - 1, y, z + dir);
        }

        // Capture diagonally right
        if (isInside(x + 1, y, z + dir) && isEnemy(x + 1, y, z + dir)) {
            Moves.emplace_back(x + 1, y, z + dir);
        }

        // Forward 1 square
        if (isInside(x, y, z + dir) && isEmpty(x, y, z + dir)) {
            Moves.emplace_back(x, y, z + dir);

            // Forward 2 squares from starting position
            if ((team == White && z == 6) || (team == Black && z == 1)) {
                if (isEmpty(x, y, z + 2 * dir)) {
                    Moves.emplace_back(x, y, z + 2 * dir);
                }
            }
        }

        break;
    }

    case Castle: {
        // Check along x and z axes in straight lines
        static const int directions[6][3] = { {1,0,0}, {-1,0,0}, {0,0,1}, {0,0,-1}, {0,1,0}, {0,-1,0} };
        for (auto& [dx, dy, dz] : directions) {
            int nx = x + dx, ny = y + dy, nz = z + dz;
            while (isInside(nx, ny, nz) && board(nx,ny,nz).getTeam() == Null) {
                Moves.emplace_back(nx, ny, nz);
                nx += dx;
                nz += dz;
                ny += dy;
            }
            if (isEnemy(nx, ny, nz)) {
                Moves.emplace_back(nx, ny, nz);
            }
        }
        break;
    }
    case Horse: {
        static const int directions[16][3] = { {2, 0, 1}, {2, 0, -1},{2, 1, 0},{2, -1, 0},
                                             {-2, 0, 1}, {-2, 0, -1},{-2, 1, 0},{-2, -1, 0},
                                             {1, 0, 2}, {-1, 0, 2},{0, 1, 2}, {0, -1, 2},
                                             {1, 0, -2}, {-1, 0, -2},{0, 1, -2}, {0, -1, -2} };
        for (auto& [dx, dy, dz] : directions) {
            if (isInside(dx + x, dy + y, dz + z) && board(dx + x,dy + y,dz + z).getTeam() != team) {
                Moves.emplace_back(dx + x, dy + y, dz + z);
            }
        }
        break;
    }
    case Bishop: {
        // Check along x and z axes in straight lines
        static const int directions[12][3] = { {1,0,1}, {-1,0,1}, {1,0,-1}, {-1,0,-1},
                                            {1,1,1}, {-1,1,1},{1,1,-1}, {-1,1,-1},
                                            {1,-1,1}, {-1,-1,1},{1,-1,-1}, {-1,-1,-1} };
        for (auto& [dx, dy, dz] : directions) {
            int nx = x + dx, ny = y + dy, nz = z + dz;
            while (isInside(nx, ny, nz) && board(nx,ny,nz).getTeam() == Null) {
                Moves.emplace_back(nx, ny, nz);
                nx += dx;
                ny += dy;
                nz += dz;

            }
            if (isEnemy(nx, ny, nz)) {
                Moves.emplace_back(nx, ny, nz);
            }
        }
        break;
    }
    case Queen: {
        // Check along x and z axes in straight lines
        static const int directions[18][3] = { {1,0,1}, {-1,0,1}, {1,0,-1}, {-1,0,-1},
                                            {1,1,1}, {-1,1,1},{1,1,-1}, {-1,1,-1},
                                            {1,-1,1}, {-1,-1,1},{1,-1,-1}, {-1,-1,-1},
                                            {1,0,0}, {-1,0,0}, {0,0,1}, {0,0,-1}, {0,1,0}, {0,-1,0} };
        for (auto& [dx, dy, dz] : directions) {
            int nx = x + dx, ny = y + dy, nz = z + dz;
            while (isInside(nx, ny, nz) && board(nx,ny,nz).getTeam() == Null) {
                Moves.emplace_back(nx, ny, nz);
                nx += dx;
                ny += dy;
                nz += dz;

            }
            if (isEnemy(nx, ny, nz)) {
                Moves.emplace_back(nx, ny, nz);
            }
        }
        break;
    }
    case King: {
        // Check along x and z axes in straight lines
        static const int directions[26][3] = {
            {-1, -1, -1}, { 0, -1, -1}, { 1, -1, -1},
            {-1,  0, -1}, { 0,  0, -1}, { 1,  0, -1},
            {-1,  1, -1}, { 0,  1, -1}, { 1,  1, -1},

            {-1, -1,  0}, { 0, -1,  0}, { 1, -1,  0},
            {-1,  0,  0},              { 1,  0,  0},
            {-1,  1,  0}, { 0,  1,  0}, { 1,  1,  0},

            {-1, -1,  1}, { 0, -1,  1}, { 1, -1,  1},
            {-1,  0,  1}, { 0,  0,  1}, { 1,  0,  1},
            {-1,  1,  1}, { 0,  1,  1}, { 1,  1,  1}
        };
        for (auto& [dx, dy, dz] : directions) {
            int nx = x + dx, ny = y + dy, nz = z + dz;
            if (isInside(nx, ny, nz) && isEmpty(nx, ny, nz)) {
                Moves.emplace_back(nx, ny, nz);
            }
            if (isInside(nx, ny, nz) && isEnemy(nx, ny, nz)) {
                Moves.emplace_back(nx, ny, nz);
            }
        }
        break;
    }
    }


    return Moves;
}

//find king position, generate all opponent moves, if a move can capture return yes
bool Chess_Logic::is_king_in_check(Array3D<Game_Piece>& board, Team team, glm::vec3& k) {
    return is_square_attacked(board, k, getOppositeTeam(team));
}

std::vector<glm::vec3> Chess_Logic::find_pieces(Array3D<Game_Piece>& board,Team t, const std::vector<Piece_Type>& types) {
    std::vector<glm::vec3> locations;
    if (types.empty()) return locations;

    // Convert input to a fast lookup set
    std::unordered_set<Piece_Type> typeSet(types.begin(), types.end());

    for (int x = 0; x < board.getWidth(); x++) {
        for (int y = 0; y < board.getHeight(); y++) {
            for (int z = 0; z < board.getDepth(); z++) {
                const Game_Piece& p = board(x,y,z);
                if (p.getTeam() == t && typeSet.count(p.getType())) {
                    locations.emplace_back(x, y, z);
                }
            }
        }
    }
    return locations;
}

std::vector<glm::vec3> Chess_Logic::getLegalMoves(Array3D<Game_Piece>& board, int x, int y, int z, Team team, glm::vec3& k) {
    std::vector<glm::vec3> legalMoves;
    std::vector<glm::vec3> candidateMoves = PossibleMoves(board, x, y, z); // pseudo-legal

    Game_Piece selectedPiece = board(x, y, z);  // Make a copy (not a reference!)
    Team opponent = (team == Team::White) ? Team::Black : Team::White;

    for (const auto& move : candidateMoves) {
        // Store the current state
        Game_Piece capturedPiece = board(move.x, move.y, move.z);
        Game_Piece originalFrom = board(x, y, z);

        glm::vec3 originalKingPos = k;
        bool isKing = (selectedPiece.getType() == King);
        if (isKing)
            k = move;

        // Simulate move
        board(move.x, move.y, move.z) = selectedPiece;
        board(x, y, z) = Game_Piece(); // Empty

        // Check if king is still safe
        bool moveIntoDanger = false;
        if (isKing && is_square_attacked(board, move, opponent)) {
            moveIntoDanger = true;
        }

        bool kingInCheck = is_king_in_check(board, team, k);

        // Undo move
        board(x, y, z) = originalFrom;
        board(move.x, move.y, move.z) = capturedPiece;

        if (isKing)
            k = originalKingPos;

        if (!kingInCheck && !moveIntoDanger)
            legalMoves.push_back(move);
    }

    return legalMoves;
}

bool Chess_Logic::is_checkMate(Array3D<Game_Piece>& board, Team team, glm::vec3& king_pos) {
    if (!is_king_in_check(board, team, king_pos)) {
        return false; // King is not in check, so not checkmate
    }

    Team opponent = (team == Team::White) ? Team::Black : Team::White;

    for (int x = 0; x < board.getWidth(); x++) {
        for (int y = 0; y < board.getHeight(); y++) {
            for (int z = 0; z < board.getDepth(); z++) {
                Game_Piece& piece = board(x, y, z);
                if (piece.getTeam() != team) continue;

                std::vector<glm::vec3> moves = getLegalMoves(board, x, y, z, team, king_pos);

                for (const glm::vec3& move : moves) {
                    // Copy board to simulate
                    Array3D<Game_Piece> tempBoard = board;

                    // Simulate the move
                    Game_Piece movingPiece = tempBoard(x, y, z);
                    tempBoard(x, y, z) = Game_Piece(); // Empty square
                    tempBoard(move.x, move.y, move.z) = movingPiece;

                    glm::vec3 new_king_pos = king_pos;
                    if (movingPiece.getType() == King) {
                        new_king_pos = move;

                        // Reject move if the king moves into an attacked square
                        if (is_square_attacked(tempBoard, new_king_pos, opponent)) {
                            continue; // illegal move — king would be captured
                        }
                    }

                    if (!is_king_in_check(tempBoard, team, new_king_pos)) {
                        return false; // At least one move escapes check safely
                    }
                }
            }
        }
    }

    return true; // No moves resolve check safely — checkmate
}


void Chess_Logic::simulateMove(Array3D<Game_Piece>& board, move m) {  


   Game_Piece movingPiece = board(m.origionalPosition.x, m.origionalPosition.y, m.origionalPosition.z);  
   board(m.newPosition.x, m.newPosition.y, m.newPosition.z) = movingPiece;  
   board(m.origionalPosition.x, m.origionalPosition.y, m.origionalPosition.z).selfDestruct();  
}

bool Chess_Logic::is_square_attacked(Array3D<Game_Piece>& board, const glm::vec3& target, Team attackerTeam) {
    int tx = (int)target.x;
    int ty = (int)target.y;
    int tz = (int)target.z;

    auto isInside = [&](int xi, int yi, int zi) {
        return xi >= 0 && xi < board.getWidth() &&
            yi >= 0 && yi < board.getHeight() &&
            zi >= 0 && zi < board.getDepth();
        };

    // === Pawn attacks ===
    int dir = (attackerTeam == White) ? -1 : 1;
    std::vector<glm::ivec3> pawnAttacks = {
        {tx + 1, ty, tz + dir},
        {tx - 1, ty, tz + dir}
    };
    for (const auto& pos : pawnAttacks) {
        if (isInside(pos.x, pos.y, pos.z)) {
            const Game_Piece& p = board(pos.x, pos.y, pos.z);
            if (p.getTeam() == attackerTeam && p.getType() == Pawn)
                return true;
        }
    }

    // === Knight attacks ===
    static const int knightMoves[][3] = {
        {2, 0, 1}, {2, 0, -1}, {2, 1, 0}, {2, -1, 0},
        {-2, 0, 1}, {-2, 0, -1}, {-2, 1, 0}, {-2, -1, 0},
        {1, 0, 2}, {-1, 0, 2}, {0, 1, 2}, {0, -1, 2},
        {1, 0, -2}, {-1, 0, -2}, {0, 1, -2}, {0, -1, -2}
    };
    for (const auto& move : knightMoves) {
        int nx = tx + move[0];
        int ny = ty + move[1];
        int nz = tz + move[2];
        if (isInside(nx, ny, nz)) {
            const Game_Piece& p = board(nx, ny, nz);
            if (p.getTeam() == attackerTeam && p.getType() == Horse) // Assuming Horse == Knight
                return true;
        }
    }

    // === Sliding pieces === (Rook, Bishop, Queen)
    static const std::vector<std::pair<Piece_Type, std::vector<glm::ivec3>>> slidingDirections = {
        {Castle, {{1,0,0}, {-1,0,0}, {0,0,1}, {0,0,-1}, {0,1,0}, {0,-1,0}}},
        {Bishop, {{1,0,1}, {-1,0,1}, {1,0,-1}, {-1,0,-1},
                  {1,1,1}, {-1,1,1}, {1,1,-1}, {-1,1,-1},
                  {1,-1,1}, {-1,-1,1}, {1,-1,-1}, {-1,-1,-1}}},
        {Queen,  {{1,0,0}, {-1,0,0}, {0,0,1}, {0,0,-1}, {0,1,0}, {0,-1,0},
                  {1,0,1}, {-1,0,1}, {1,0,-1}, {-1,0,-1},
                  {1,1,1}, {-1,1,1}, {1,1,-1}, {-1,1,-1},
                  {1,-1,1}, {-1,-1,1}, {1,-1,-1}, {-1,-1,-1}}}
    };

    for (const auto& [pieceType, directions] : slidingDirections) {
        for (const glm::ivec3& dir : directions) {
            int nx = tx + dir.x;
            int ny = ty + dir.y;
            int nz = tz + dir.z;
            while (isInside(nx, ny, nz)) {
                const Game_Piece& p = board(nx, ny, nz);
                if (p.getTeam() == Null) {
                    nx += dir.x;
                    ny += dir.y;
                    nz += dir.z;
                    continue;
                }
                if (p.getTeam() == attackerTeam &&
                    (p.getType() == pieceType || p.getType() == Queen)) {
                    return true;
                }
                break;
            }
        }
    }

    // === King threats (adjacent 26 directions) ===
    static const int kingMoves[][3] = {
        {-1, -1, -1}, { 0, -1, -1}, { 1, -1, -1},
        {-1,  0, -1}, { 0,  0, -1}, { 1,  0, -1},
        {-1,  1, -1}, { 0,  1, -1}, { 1,  1, -1},

        {-1, -1,  0}, { 0, -1,  0}, { 1, -1,  0},
        {-1,  0,  0},              { 1,  0,  0},
        {-1,  1,  0}, { 0,  1,  0}, { 1,  1,  0},

        {-1, -1,  1}, { 0, -1,  1}, { 1, -1,  1},
        {-1,  0,  1}, { 0,  0,  1}, { 1,  0,  1},
        {-1,  1,  1}, { 0,  1,  1}, { 1,  1,  1}
    };
    for (const auto& move : kingMoves) {
        int nx = tx + move[0];
        int ny = ty + move[1];
        int nz = tz + move[2];
        if (isInside(nx, ny, nz)) {
            const Game_Piece& p = board(nx, ny, nz);
            if (p.getTeam() == attackerTeam && p.getType() == King)
                return true;
        }
    }

    return false;
}

bool Chess_Logic::is_staleMate(Array3D<Game_Piece>& board, Team team, glm::vec3 k) {

    for (int x = 0; x < board.getWidth(); x++) {
        for (int y = 0; y < board.getHeight(); y++) {
            for (int z = 0; z < board.getDepth(); z++) {
                if (board(x, y, z).getTeam() == team) {
                    auto legal = getLegalMoves(board, x, y, z, team, k);
                    if (!legal.empty()) return false;
                }
            }
        }
    }

    return true;
}
