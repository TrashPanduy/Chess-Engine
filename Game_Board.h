#pragma once  
#include "Chess_Logic.h"
#include <stack>
#include "board_class.h"



class Game_Board : public VulkanHelper { // Added 'public' inheritance specifier. 
protected:
    Chess_Logic logic;
private:  
   int width, height, depth;  
   bool check = false;
   Array3D<Game_Piece> board;
   std::unordered_map<Piece_Type, ModelLoader> pieceModels;
   std::unordered_map<std::pair<Piece_Type, Team>, TextureImage, PairHash> pieceTextures;
   glm::vec3 boardPosition = glm::vec3(0.0f, 0.0f, -5.0f);
   glm::vec3 boardRotation = glm::vec3(45.0f, 0.0f, 90.0f);
   glm::mat4 transform;
   std::vector<glm::vec3> ghostList;

   std::thread aiThread;
   std::atomic<bool> aiThinking = false;
   std::atomic<bool> aiCancelled = false;
   bool AI_Active = false;

   std::future<move> aiFuture;

   VkDevice device;
   VkPhysicalDevice pDevice;
   VkCommandPool cPool;
   VkQueue queue;

   glm::vec3 whiteKingPos;
   glm::vec3 blackKingPos;
    
   Game_Piece* selectedPiece = nullptr;
   glm::vec3 selectedPiecePosition;

   Team activePlayer = White;
   Game_Piece createPiece(Piece_Type type, Team team, glm::vec3 piecePosition, glm::vec3 pieceRotation);

   void loadModels();
   void loadTextures();
   void populateBoard();

   void movePiece(move m);
   void startAIThinking(GameState& gameState);



   class Chess_Bot {
   public:
       uint32_t ttHits, ttMisses;
       //Zobrist Hashing
       class Zobrist {
       public:
           static constexpr int NUM_PIECES = 13; // Number of (piece × team) types
           inline static uint64_t sideToMoveHash = 0;

           inline static std::vector<std::vector<std::vector<std::vector<uint64_t>>>> table;

           static void init(int width, int height, int depth); // Call this once at startup
           static uint64_t computeHash(Array3D<Game_Piece>& board);
           static int pieceIndex(Piece_Type type, Team team) {
               if (team == Null || type == None) return -1;
               return static_cast<int>(type) + (team == White ? 0 : 6); // Adjust based on layout
           }
           static uint64_t updateHash(uint64_t currentHash, const move& m);
           static uint64_t revertHash(uint64_t currentHash, const move& m);
           static uint64_t nullMoveHash(uint64_t currentHash, Team team) {
               return currentHash ^ sideToMoveHash;
           }
           static uint64_t revertNullMoveHash(uint64_t currentHash, Team team) {
               return currentHash ^ sideToMoveHash;
           }


       };
   private:

       //temporary variables for debugging
       int totalGeneratedMoves = 0;
       int totalMoveGenCalls = 0;
       //end here

       Team Bot_Team;
       int plies;
       Array3D<Game_Piece> board_state;
       static enum move_types{
           Check,
           Check_Mate,
           Take_Piece,
           Lose_Piece,
           Protect_Piece
       };
       std::unordered_map<Piece_Type, int> pieceValues = {
           {Pawn,   1},
           {Horse, 3},
           {Bishop, 3},
           {Castle,   5},
           {Queen,  9},
           {King,   0}  
       };
       std::unordered_map<move_types, int> moveValues = {
           {Check,  25},
           {Check_Mate, 1000}
       };
       int find_board_quality(move m);
       int evaluateBoard(Array3D<Game_Piece>& b, Team perspective);
       int evaluatePiece(const Game_Piece& p, int x, int y, int z, Array3D<Game_Piece>& board);
       Game_Piece simulateMoveInPlace(Array3D<Game_Piece>& board, const move& m);
       void undoMove(Array3D<Game_Piece>& board, const move& m, const Game_Piece& captured);
       int quiescenceSearch(Array3D<Game_Piece>& board, int alpha, int beta, Team team, uint64_t hash,glm::vec3& k,int qDepth);
       void stripRenderingData(Array3D<Game_Piece>& board);
       struct MoveRecord {
           move m;
           Game_Piece captured;
           uint64_t hashBeforeMove;
       };
       std::stack<MoveRecord> moveHistory;
       uint64_t currentHash;
       glm::vec3 whiteKingPos;
       glm::vec3 blackKingPos;
       static const uint16_t MAX_DEPTH = 16;
       std::array<std::array<move, 2>, MAX_DEPTH> killerMoves;
       int historyHeuristic[7][8][3][8] = {}; // [piece][x][y][z]
       int nodesSearched = 0;
       std::vector<move> getAllLegalMoves(Array3D<Game_Piece>& board, Team t, glm::vec3& kingPos);
       const static int32_t startingAlpha = -30000;
       const static int32_t startingBeta = 30000;
       int numFutilityCandidates = 0;
       uint64_t evalScore;
       enum TTFlag {  
           EXACT,  
           LOWERBOUND,  
           UPPERBOUND  
        };
       struct TTEntry {
           uint64_t hash = 0;
           int depth = -1;
           int score = 0;
           TTFlag flag;
           move bestMove;
       };


       //Class for Transposition Table
       class TranspositionTable {
           static constexpr size_t TABLE_SIZE = 1 << 20; // 1 million entries approx
           std::vector<TTEntry> table;


       public:
           int alphaBeta(Chess_Bot& bot, Array3D<Game_Piece>& board, int depth, int alpha, int beta, Team team, TranspositionTable& tt);
           TranspositionTable() : table(TABLE_SIZE) {}

           void store(uint64_t hash, int depth, int score, TTFlag flag, move bestMove) {
               size_t index = hash % TABLE_SIZE;
               TTEntry& entry = table[index];
               // Replace only if new depth is deeper or slot is empty
               if (entry.depth < depth) {
                   entry.hash = hash;
                   entry.depth = depth;
                   entry.score = score;
                   entry.flag = flag;
                   entry.bestMove = bestMove;
               }
           }

           TTEntry* probe(uint64_t hash) {
               size_t index = hash % TABLE_SIZE;
               TTEntry& entry = table[index];
               if (entry.hash == hash)
                   return &entry;
               return nullptr;
           }
       };

   public:
       Chess_Bot(int plies, Team team, int width, int height, int depth) {
           this->plies = plies;
           this->Bot_Team = team;
           //call ONCE on startup, for optimization of Chess_Bot.
           Chess_Bot::Zobrist::init(width,height,depth);
       }
       Chess_Bot() {
           Bot_Team = Null;
           plies = 2;
           Chess_Bot::Zobrist::init(8, 3, 8);
       }
       Team getTeam() {
           return Bot_Team;
       }
       void setTeam(Team t) {
           Bot_Team = t;
       }
       move calculateMove(Array3D<Game_Piece> board,Team t, std::atomic<bool>& aiCancelled);
       TranspositionTable transpositionTable;

       void updateKingPosition(glm::vec3 w, glm::vec3 b) {
           whiteKingPos = w;
           blackKingPos = b;
       }
   };

   Chess_Bot ChessBot;
   Team AI_Team;
   //make a deep copy

public:  
   ModelLoader boardModel;
   TextureImage boardTexture;
   Game_Board(int w, int h, int d, VkDevice device, VkPhysicalDevice p, VkCommandPool c, VkQueue q,Team AI_Team) {
       width = w;  
       height = h;  
       depth = d;  
       board = Array3D<Game_Piece>(width, height, depth);
       board.resize(width, height, depth);
       this->device = device;
       this->pDevice = p;
       this->cPool = c;
       this->queue = q;
       this->boardModel = ModelLoader(device, pDevice, cPool, queue);
       this->boardTexture = TextureImage();
       this->AI_Team = AI_Team;
       boardModel.Load(BOARD_MODEL_PATH.c_str());
       boardTexture.Load(BOARD_TEXTURE_PATH.c_str());
       loadModels();
       loadTextures();
       populateBoard();

   }
   Game_Board() {
       width = 8;
       height = 3;
       depth = 8;
       board.resize(width, height, depth);


   }
   void startupAssignVars_FuckThreads(int w, int h, int d, VkDevice device, VkPhysicalDevice p, VkCommandPool c, VkQueue q, Team AI_Team) {
       // clean up old resources if needed

       width = w;
       height = h;
       depth = d;
       this->device = device;
       this->pDevice = p;
       this->cPool = c;
       this->queue = q;
       this->AI_Team = AI_Team;
       this->boardModel = ModelLoader(device, pDevice, cPool, queue);
       this->boardTexture = TextureImage();
       boardModel.Load(BOARD_MODEL_PATH.c_str());
       boardTexture.Load(BOARD_TEXTURE_PATH.c_str());
       loadModels();
       loadTextures();
       populateBoard();
   }
   void checkAIAndApplyMove();
   void cleanUpModels();
   void cleanUpTextures();
   void renderModels(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, std::vector<VkDescriptorSet> descriptorSets, uint32_t currentFrame);
   void renderHitboxes(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout);
   void updateTransform(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout);
   void renderGhosts(std::vector<glm::vec3> Locations);
   void deleteGhostList(std::vector<glm::vec3> list);
   Team Who_Tf_Won();
   void resetBoard();

   void updatePieceTexture(Game_Piece* gp) {
       gp->setTexture(&pieceTextures[{gp->getType(), gp->getTeam()}]);
   }
   //getters and setters
   glm::vec3 getBoardRotation() const {
       return boardRotation;
   }
   void setBoardRotation(const glm::vec3& rotation) {
       boardRotation = rotation;
   }
   glm::vec3 getBoardPosition(){
       return boardPosition;
   }
   void setBoardPosition(glm::vec3 position) {
       boardPosition = position;
   }
   glm::mat4 getBoardTransform(){
       return transform;
   }
   void setBoardTransform(glm::mat4& t) {
       transform = t;
   }
   Array3D<Game_Piece>& getBoard() {
       return board; 
   }
   Game_Piece* getSelectedPiece() {
       return selectedPiece;
   }
   Team getActivePlayer() const {
       return activePlayer;
   }
   void setActivePlayer(Team player) {
       activePlayer = player;
   }
   Team getOppositePlayer(Team team) {
       if (team == White) { return Black; }
       else { return White; }
       return Null;
   }
   void setAITeam(Team AI) {
       if (AI == White || AI == Black) {
           AI_Team = AI;
           ChessBot = Chess_Bot(4, AI,width,height,depth);
       }
   }
   void cancelAI() {
       aiCancelled = true;
   }
   void setAiMode(bool mode) {
       AI_Active = mode;
   }
   //OnClick start
   void Update_Game_Board(Game_Piece* gp, int x, int y, int z, GameState& gameState);
}; 