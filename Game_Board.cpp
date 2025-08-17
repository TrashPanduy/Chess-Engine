//By Michael Owen
// This file is dedicated to implementing Game_Board functions defined in Game_Board.h
// And support functions from class Game_Piece
// Last updated 6/8/2025
#include "Game_Board.h"
#include <unordered_set>

/*
 * Populates the Game_Board::board data structure with the initial arrangement of chess pieces.
 * Currently supports only a 2D layout; 3D functionality to be implemented.
 */
void Game_Board::populateBoard() {
    auto placePiece = [&](Piece_Type type, Team team, glm::ivec3 pos, glm::vec3 rotation) {
        board(pos.x, pos.y, pos.z) = createPiece(type, team, pos, rotation);
        };

    auto placeBackRank = [&](int z, Team team, glm::vec3 rotation) {
        int y = 1;

        placePiece(Castle, team, { 0, y, z }, rotation);
        placePiece(Horse, team, { 1, y, z }, rotation);
        placePiece(Bishop, team, { 2, y, z }, rotation);
        placePiece(King, team, { 3, y, z }, rotation);
        placePiece(Queen, team, { 4, y, z }, rotation);
        placePiece(Bishop, team, { 5, y, z }, rotation);
        placePiece(Horse, team, { 6, y, z }, rotation);
        placePiece(Castle, team, { 7, y, z }, rotation);
        };

    auto placeOuterRanks = [&](int y) {
        for (int x = 0; x < width; ++x) {
            placePiece(Pawn, Team::Black, { x, y, 1 }, { 0.0f, 0.0f, 0.0f });
            placePiece(Pawn, Team::White, { x, y, 6 }, { 180.0f, 0.0f, 0.0f });
        }

        for (int z : { 0, 7 }) {
            Team team = (z == 0) ? Black : White;
            glm::vec3 rotation = (team == Black) ? glm::vec3(0.0f) : glm::vec3(180.0f, 0.0f, 0.0f);

            placePiece(Pawn, team, { 0, y, z }, rotation);
            placePiece(Castle, team, { 1, y, z }, rotation);
            placePiece(Horse, team, { 2, y, z }, rotation);
            placePiece(Bishop, team, { 3, y, z }, rotation);
            placePiece(Bishop, team, { 4, y, z }, rotation);
            placePiece(Horse, team, { 5, y, z }, rotation);
            placePiece(Castle, team, { 6, y, z }, rotation);
            placePiece(Pawn, team, { 7, y, z }, rotation);
        }
        };

    // Place back rank on middle layer
    placeBackRank(0, Black, { 0.0f, 0.0f, 0.0f });
    placeBackRank(7, White, { 180.0f, 0.0f, 0.0f });

    // Save king positions
    blackKingPos = { 3, 1, 0 };
    whiteKingPos = { 3, 1, 7 };

    // Place outer layer pawns and side ranks
    for (int y : { 0, 2 }) {
        placeOuterRanks(y);
    }

    // Extra central pawns
    for (int x = 0; x < width; ++x) {
        placePiece(Pawn, Team::Black, { x, 1, 1 }, { 0.0f, 0.0f, 0.0f });
        placePiece(Pawn, Team::White, { x, 1, 6 }, { 180.0f, 0.0f, 0.0f });
    }
}


/*
 * Implements Game_Board::loadModels().
 * Generates a ModelLoader instance for each chess piece type
 * and loads the corresponding model from a hardcoded path.
 */
void Game_Board::loadModels() {
    // Load models for each piece type
    pieceModels[Piece_Type::Pawn] = ModelLoader(device, pDevice, cPool, queue);
    pieceModels[Piece_Type::Castle] = ModelLoader(device, pDevice, cPool, queue);
    pieceModels[Piece_Type::Bishop] = ModelLoader(device, pDevice, cPool, queue);
    pieceModels[Piece_Type::Horse] = ModelLoader(device, pDevice, cPool, queue);
    pieceModels[Piece_Type::Queen] = ModelLoader(device, pDevice, cPool, queue);
    pieceModels[Piece_Type::King] = ModelLoader(device, pDevice, cPool, queue);
    pieceModels[Piece_Type::Pawn].Load("models/Pawn.obj");
    pieceModels[Piece_Type::Castle].Load("models/Castle.obj");
    pieceModels[Piece_Type::Bishop].Load("models/Bishop.obj");
    pieceModels[Piece_Type::Horse].Load("models/Horse.obj");
    pieceModels[Piece_Type::Queen].Load("models/Queen.obj");
    pieceModels[Piece_Type::King].Load("models/King.obj");
}

/*
 * Implements Game_Board::loadTextures().
 * Generates a textureImage and loads it for use with specific pieceTextures combination
 * Images from hardcoded path.
 */
void Game_Board::loadTextures() {
    // Load textures by team + type
    pieceTextures[{Piece_Type::Pawn, Team::White}].Load("textures/SilverPawn.png");
    pieceTextures[{Piece_Type::Pawn, Team::Black}].Load("textures/PawnGold.png");
    pieceTextures[{Piece_Type::Castle, Team::White}].Load("textures/SilverCastle.png");
    pieceTextures[{Piece_Type::Castle, Team::Black}].Load("textures/CastleGold.png");
    pieceTextures[{Piece_Type::Bishop, Team::White}].Load("textures/SilverBishop.png");
    pieceTextures[{Piece_Type::Bishop, Team::Black}].Load("textures/BishopGold.png");
    pieceTextures[{Piece_Type::Horse, Team::White}].Load("textures/SilverHorse.png");
    pieceTextures[{Piece_Type::Horse, Team::Black}].Load("textures/HorseGold.png");
    pieceTextures[{Piece_Type::Queen, Team::White}].Load("textures/SilverQueen.png");
    pieceTextures[{Piece_Type::Queen, Team::Black}].Load("textures/QueenGold.png");
    pieceTextures[{Piece_Type::King, Team::White}].Load("textures/SilverKing.png");
    pieceTextures[{Piece_Type::King, Team::Black}].Load("textures/KingGold.png");
    
    pieceTextures[{Piece_Type::Pawn, Team::WhiteSelected}].Load("textures/PawnBlue.png");
    pieceTextures[{Piece_Type::Castle, Team::WhiteSelected}].Load("textures/CastleBlue.png");
    pieceTextures[{Piece_Type::Bishop, Team::WhiteSelected}].Load("textures/BishopBlue.png");
    pieceTextures[{Piece_Type::Horse, Team::WhiteSelected}].Load("textures/HorseBlue.png");
    pieceTextures[{Piece_Type::Queen, Team::WhiteSelected}].Load("textures/QueenBlue.png");
    pieceTextures[{Piece_Type::King, Team::WhiteSelected}].Load("textures/KingBlue.png");

    pieceTextures[{Piece_Type::Pawn, Team::BlackSelected}].Load("textures/PawnGreen.png");
    pieceTextures[{Piece_Type::Castle, Team::BlackSelected}].Load("textures/CastleGreen.png");
    pieceTextures[{Piece_Type::Bishop, Team::BlackSelected}].Load("textures/BishopGreen.png");
    pieceTextures[{Piece_Type::Horse, Team::BlackSelected}].Load("textures/HorseGreen.png");
    pieceTextures[{Piece_Type::Queen, Team::BlackSelected}].Load("textures/QueenGreen.png");
    pieceTextures[{Piece_Type::King, Team::BlackSelected}].Load("textures/KingGreen.png");

    pieceTextures[{Piece_Type::Pawn, Team::Ghost}].Load("textures/PawnWhite.png");
    pieceTextures[{Piece_Type::Castle, Team::Ghost}].Load("textures/CastleWhite.png");
    pieceTextures[{Piece_Type::Bishop, Team::Ghost}].Load("textures/BishopWhite.png");
    pieceTextures[{Piece_Type::Horse, Team::Ghost}].Load("textures/HorseWhite.png");
    pieceTextures[{Piece_Type::Queen, Team::Ghost}].Load("textures/QueenWhite.png");
    pieceTextures[{Piece_Type::King, Team::Ghost}].Load("textures/KingWhite.png");

    pieceTextures[{Piece_Type::Pawn, Team::Target}].Load("textures/PawnRed.png");
    pieceTextures[{Piece_Type::Castle, Team::Target}].Load("textures/CastleRed.png");
    pieceTextures[{Piece_Type::Bishop, Team::Target}].Load("textures/BishopRed.png");
    pieceTextures[{Piece_Type::Horse, Team::Target}].Load("textures/HorseRed.png");
    pieceTextures[{Piece_Type::Queen, Team::Target}].Load("textures/QueenRed.png");
    pieceTextures[{Piece_Type::King, Team::Target}].Load("textures/KingRed.png");
}

//Implements Game_Board::cleanUpModels(). Destroy all stored models in pieceModels data structure
void Game_Board::cleanUpModels() {
    for (auto& [type, model] : pieceModels) {
        model.Destroy();
    }
}
//Implements Game_Board::cleanUpTextures(). Destroy all stored textures in pieceTextures data structure
void Game_Board::cleanUpTextures() {
    for (auto& [key, texture] : pieceTextures) {
        texture.Destroy();
    }
}

/**
 * @brief Creates and initializes a Game_Piece object with the appropriate model, texture,
 *        transform, and team/type data.
 *
 * @param type ; Enum Piece_Type, holds type of piece or None at all.(e.g., Castle, Horse, Pawn...)
 * @param team ; Enum Team, The team to which the piece belongs (e.g., Black or White)
 * @param modelPosition ; glm::vec3 ; The position of the piece in board coordinates (x, y, z)
 * @param modelRotation ; glm::vec3 ; The rotation of the model in degrees around each axis (x, y, z)
 *
 * @return A fully initialized Game_Piece object with transforms applied
 */
Game_Piece Game_Board::createPiece(Piece_Type type, Team team, glm::vec3 modelPosition, glm::vec3 modelRotation) {

    // Retrieve the model and texture associated with the piece type and team
    ModelLoader* model = &pieceModels[type];
    TextureImage* texture = &pieceTextures[{type, team}];

    // Ensure model and texture pointers are valid
    assert(model != nullptr && "ModelLoader pointer is null");
    assert(texture != nullptr && "TextureImage pointer is null");

    //Create the Game Piece as variable piece
    Game_Piece piece(model, texture, team, type);


    // Initialize transform matrix
    glm::mat4 localTransform = glm::mat4(1.0f);

    int offset = (team == White) ? 0 : 2;
    //offset values: -6.8
    glm::vec3 adjustedPosition((modelPosition[0] * 1.945f) - 6.8, (modelPosition[1]*2.6) + 0.37, ((modelPosition[2] - offset) * 1.945f) - 4.8);
    localTransform = glm::translate(localTransform, adjustedPosition);

    // Apply rotations
    localTransform = glm::rotate(localTransform, glm::radians(modelRotation.z), glm::vec3(0, 0, 1));
    localTransform = glm::rotate(localTransform, glm::radians(modelRotation.y), glm::vec3(1, 0, 0));
    localTransform = glm::rotate(localTransform, glm::radians(modelRotation.x), glm::vec3(0, 1, 0));

    //Apply uniform scale
    localTransform = glm::scale(localTransform, glm::vec3(1.5f));

    // Set local transform and update world transform based on board's transform
    //Essentially child game piece to game board
    piece.setLocalTransform(localTransform);
    piece.updateWorldTransform(getBoardTransform());

    if (debugMode) {
        // === Hitbox creation ===
        ModelLoader hitbox(device, pDevice, cPool, queue);
        hitbox.setAABBMin(model->getAABBMin());
        hitbox.setAABBMax(model->getAABBMax());
        hitbox.GenerateAABBCube(glm::vec3(1.0f, 0.0f, 0.0f)); // Red wireframe debug box
        piece.setHitboxModel(hitbox);
    }

    return piece;
}

/**
 * @brief Renders all active chess piece models on the board using Vulkan commands.
 *
 * This function iterates through the back two ranks (z = 0, 1, 6, 7) of the board
 * and renders each Game_Piece if it has a valid model and texture.
 *
 * @param commandBuffer     The Vulkan command buffer used to issue rendering commands.
 * @param pipelineLayout    The Vulkan pipeline layout, used for push constants and descriptor sets.
 * @param descriptorSets    The descriptor sets used for global shader resources.
 * @param currentFrame      The current frame index.
 */
void Game_Board::renderModels(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout,std::vector<VkDescriptorSet> descriptorSets,uint32_t currentFrame) {
    // Only render pieces in the first and last two ranks of the board for now
        for (int x = 0; x < board.getWidth(); x++){
            for (int y = 0; y < board.getHeight(); y++) {
                for (int z = 0; z < board.getDepth(); z++) {
                    Game_Piece& piece = board(x,y,z);
                    if (piece.getTeam() != Null) {
                        ModelLoader* model = piece.getModel();
                        TextureImage* texture = piece.getTexture();

                        // Ensure both model and texture are valid before rendering
                        if (model && model->vertexBuffer != VK_NULL_HANDLE && piece.getTexture()->descriptorSet) {
                            assert(pipelineLayout != VK_NULL_HANDLE && "pipelineLayout is null!");
                            assert(commandBuffer != VK_NULL_HANDLE);

                            // Bind the descriptor set associated with this piece's texture
                            vkCmdBindDescriptorSets(
                                commandBuffer,
                                VK_PIPELINE_BIND_POINT_GRAPHICS,
                                pipelineLayout,
                                1,
                                1,
                                &piece.getTexture()->descriptorSet,
                                0,
                                nullptr);

                            // Push the world transform matrix for this piece as a push constant
                            glm::mat4 transform = piece.getWorldTransform();
                            vkCmdPushConstants(
                                commandBuffer,
                                pipelineLayout,
                                VK_SHADER_STAGE_VERTEX_BIT,
                                0,
                                sizeof(glm::mat4),
                                &transform
                            );

                            // Issue draw command using the model's vertex/index buffers
                            model->Render(commandBuffer);
                            // Draw debug hitbox
                            if (debugMode) {
                                ModelLoader& hitbox = piece.getHitboxModel();
                                if (hitbox.vertexBuffer != VK_NULL_HANDLE) {
                                    // Push the same transform as the piece
                                    vkCmdPushConstants(
                                        commandBuffer,
                                        pipelineLayout,
                                        VK_SHADER_STAGE_VERTEX_BIT,
                                        0,
                                        sizeof(glm::mat4),
                                        &transform
                                    );
                                    hitbox.Render(commandBuffer);
                                }
                            }
                    }
                    

                    }
                }
            }
        }
}

/**
 * @brief Updates the transformation matrix of the board and applies it to each relevant game piece.
 *
 * This function recalculates the world-space transform of the board based on its current position,
 * rotation, and scale. It then updates the world transform of all active game pieces that require it.
 * Finally, the board's transform is pushed as a constant to the GPU pipeline.
 *
 * @param commandBuffer   The Vulkan command buffer used to record GPU commands.
 * @param pipelineLayout  The Vulkan pipeline layout, required to push constants to the shaders.
 */
void Game_Board::updateTransform(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout) {
    // Compute the board's global transformation matrix
    glm::mat4 transform = glm::mat4(1.0f);

    // Apply translation
    transform = glm::translate(transform, getBoardPosition());

    // Apply Z-X-Y rotation (applied in this order due to matrix multiplication sequence)
    transform = glm::rotate(transform, glm::radians(getBoardRotation()[2]), glm::vec3(0.0f, 0.0f, 1.0f)); //z-rotation
    transform = glm::rotate(transform, glm::radians(getBoardRotation()[0]), glm::vec3(1.0f, 0.0f, 0.0f)); //x-rotation
    transform = glm::rotate(transform, glm::radians(getBoardRotation()[1]), glm::vec3(0.0f, 1.0f, 0.0f)); //y-rotation

    // Apply game board scaling
    transform = glm::scale(transform, glm::vec3(0.7f));

    // Store the calculated board transform in Game_Board::transform
    setBoardTransform(transform);

    // Update the world transform for each game piece, limited to only back and front 2 rows for now
    for (int y = 0; y < board.getHeight(); y++) {
        for (int z = 0; z < board.getDepth(); z++) {
            for (int x = 0; x < board.getWidth(); x++) {
                Game_Piece& piece = board(x,y,z);
                // This must be called every frame to keep transforms in sync with board movement
                piece.updateWorldTransform(getBoardTransform());
            }
        }
    }

    // Push the final board transform matrix to the vertex shader via push constants
    vkCmdPushConstants(
        commandBuffer,
        pipelineLayout,
        VK_SHADER_STAGE_VERTEX_BIT,
        0,
        sizeof(glm::mat4),
        &transform
    );
}
void Game_Board::renderHitboxes(VkCommandBuffer commandBuffer, VkPipelineLayout debugPipelineLayout) {
    for (int y = 0; y < 3; y++) {
        for (int z : {0, 1, 6, 7}) {
            for (int x = 0; x < width; x++) {
                Game_Piece& piece = board(x,y,z);

                // Get the AABB cube model
                ModelLoader& hitbox = piece.getHitboxModel();

                if (hitbox.vertexBuffer != VK_NULL_HANDLE) {
                    glm::mat4 modelTransform = piece.getWorldTransform();

                    // Push model transform for hitbox
                    vkCmdPushConstants(
                        commandBuffer,
                        debugPipelineLayout,
                        VK_SHADER_STAGE_VERTEX_BIT,
                        0,
                        sizeof(glm::mat4),
                        &modelTransform
                    );

                    // Draw the hitbox
                    hitbox.Render(commandBuffer);
                }
            }
        }
    }
}

void Game_Board::renderGhosts(std::vector<glm::vec3> locations) {
    glm::vec3 rotation = (selectedPiece->getTeam() == White) ? glm::vec3(180.0f, 0.0f, 0.0f) : glm::vec3(0.0f, 0.0f, 0.0f);

    for (const auto& pos : locations) {
        int x = pos.x;
        int y = pos.y;
        int z = pos.z;
        if (board(x,y,z).getTeam() == Null) {
            board(x,y,z) = createPiece(selectedPiece->getType(), Ghost, pos, rotation);
        }
        else {
            board(x,y,z) = createPiece(board(x,y,z).getType(), Target, pos, rotation);
        }

    }
}

void Game_Board::deleteGhostList(std::vector<glm::vec3> locations) {
    for (const auto& pos : locations) {
        Game_Piece* piece = &board(pos.x,pos.y,pos.z);
        if (piece->getTeam() == Ghost) {
            piece->selfDestruct();
        }
        else if(piece->getTeam() == Target){
            if (selectedPiece->getTeam() == WhiteSelected) {
                piece->setTeam(Black);
            }
            else { piece->setTeam(White); }
            updatePieceTexture(piece);
        }
    }
}

Team Game_Board::Who_Tf_Won() {
    if (Chess_Logic::is_checkMate(board, White,whiteKingPos)) {
        return Black;
    }
    if (Chess_Logic::is_checkMate(board, Black,blackKingPos)) {
        return White;
    }
    return Null;
}

void Game_Board::Update_Game_Board(Game_Piece* gp, int x, int y, int z, GameState& gameState) {
   
    //check if a piece has been selected already
    if (selectedPiece) {
        //check if the newly selected piece is a Ghost or Target. If so move og piece to new location
        if (gp->getTeam() == Ghost || gp->getTeam() == Target) {
            if (aiThinking) return;

            std::cout << "Making Move: " << aiThinking << std::endl;
            Team team = Null;
            if (selectedPiece->getTeam() == WhiteSelected) {
                team = White;
            }
            else if (selectedPiece->getTeam() == BlackSelected) {
                team = Black;
            }
            move m = move(team, board(selectedPiecePosition.x,selectedPiecePosition.y,selectedPiecePosition.z).getType(), selectedPiecePosition, { x,y,z }, None);
            deleteGhostList(ghostList); 
            movePiece(m);

        
            //update textures
            activePlayer = (team == White) ? Black : White;
            selectedPiece = nullptr;
            selectedPiecePosition = {};
            auto& kingPos = (activePlayer == Black) ? blackKingPos : whiteKingPos;

            //check for checkmate
            if (logic.is_checkMate(board,activePlayer, kingPos)) {
                std::cout << "ending game" << std::endl;
                //Load a screen that either prompts a rematch OR exits to main menu
                gameState = End_Screen;
            }
        }
        //if new and og selected pieces are the same empty selected status
        else if (selectedPiecePosition == glm::vec3{ x,y,z }) {
            deleteGhostList(ghostList);
            selectedPiece->setTeam(activePlayer);
            updatePieceTexture(selectedPiece);
            selectedPiece = nullptr;
        }
        else {
            deleteGhostList(ghostList);
            selectedPiece->setTeam(activePlayer);
            updatePieceTexture(selectedPiece);
            selectedPiece = nullptr;


            //replace selectedPiece
            selectedPiece = gp;
            selectedPiecePosition = { x, y, z };

            //get possible moves for selected gamePiece
            auto& kingPos = (activePlayer == Black) ? blackKingPos : whiteKingPos;
            ghostList = logic.getLegalMoves(board,x, y, z, activePlayer, kingPos);

            if (gp->getTeam() == White) {
                selectedPiece->setTeam(WhiteSelected);
            }
            else {
                selectedPiece->setTeam(BlackSelected);
            }
            //update selected piece's texture so renderer renders propper shit
            updatePieceTexture(selectedPiece);
            //render possible movements to board as Ghost objects
            renderGhosts(ghostList);
        }
    }
    else {
        //replace selectedPiece
        selectedPiece = gp;
        selectedPiecePosition = { x, y, z };

        //get possible moves for selected gamePiece
        auto& kingPos = (activePlayer == Black) ? blackKingPos : whiteKingPos;
        ghostList = logic.getLegalMoves(board, x, y, z, activePlayer, kingPos);

        if (!aiThinking) {
            if (gp->getTeam() == White) {
                selectedPiece->setTeam(WhiteSelected);
            }
            else {
                selectedPiece->setTeam(BlackSelected);
            }
        }
        //update selected piece's texture so renderer renders propper shit
        updatePieceTexture(selectedPiece);
        //render possible movements to board as Ghost objects
        renderGhosts(ghostList);

    }
    if (activePlayer == AI_Team && AI_Active) {
        startAIThinking(gameState);
    }

    //check possible End conditions
    if (Chess_Logic::is_checkMate(board, White, whiteKingPos)) {
        gameState = End_Screen;
    }
    if (Chess_Logic::is_checkMate(board, Black, blackKingPos)) {
        gameState = End_Screen;
    }
    if (Chess_Logic::is_staleMate(board,White,whiteKingPos)) {
        gameState = End_Screen;
    }
    if (Chess_Logic::is_staleMate(board, Black, blackKingPos)) {
        gameState = End_Screen;
    }
}
void Game_Board::movePiece(move m) {
    if (m.pieceType == King) {
        if (m.team == White) {
            whiteKingPos = m.newPosition;
        }
        else {
            blackKingPos = m.newPosition;
        }
    }


    assert(m.pieceType > 0 && m.pieceType <= Castle);

    glm::vec3 rotation = (m.team == White) ? glm::vec3(180.0f, 0.0f, 0.0f) : glm::vec3(0.0f, 0.0f, 0.0f);
    board(m.newPosition.x,m.newPosition.y,m.newPosition.z) = createPiece(m.pieceType, m.team, m.newPosition, rotation);
    board(m.origionalPosition.x,m.origionalPosition.y,m.origionalPosition.z).selfDestruct();
    updatePieceTexture(&board(m.newPosition.x,m.newPosition.y,m.newPosition.z));
    if (AI_Team == Black) {
        ChessBot.updateKingPosition(whiteKingPos, blackKingPos);
    }
}

void Game_Board::resetBoard() {
    vkDeviceWaitIdle(device);

    board = Array3D<Game_Piece>(width, height, depth);
    ghostList.clear();
    selectedPiece = nullptr;
    activePlayer = White;

    populateBoard(); // Reset piece positions and game logic
}

void Game_Board::startAIThinking(GameState& gameState) {
    
    if (aiThinking.load()) return; // AI already thinking, ignore

    aiThinking.store(true);
    aiCancelled.store(false);

    // Launch async task to calculate move
    aiFuture = std::async(std::launch::async, [this]() -> move {
        // calculateMove might take time, so run it async
        return ChessBot.calculateMove(board, AI_Team, aiCancelled);
    });
    
}

void Game_Board::checkAIAndApplyMove() {
    if (aiThinking.load()) {
        if (aiFuture.wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
            move aiMove = aiFuture.get();  // get the result
            if (!aiCancelled.load()) {
                deleteGhostList(ghostList);
                selectedPiece = nullptr;

                movePiece(aiMove);              // apply the move on the board
                setActivePlayer(getOppositePlayer(AI_Team));  // switch turn
            }

            aiThinking.store(false);       // AI done thinking
        }
    }

}