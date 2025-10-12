#pragma once
#include <vector>
#include "ModelLoader.h"
#include "TextureImage.h"
#include <functional> // for std::hash
#include <utility>    // for std::pair
#include <unordered_set>
#include <random>
#include <ctime>


//custom hashmap to relate 2 classes together with a custom seed
//Use Case - correlate Team and Piece_Type to organize dedicated models and textures
struct PairHash {
    template <class T1, class T2>
    std::size_t operator()(const std::pair<T1, T2>& p) const {
        std::size_t seed = 0;
        seed ^= std::hash<T1>{}(p.first) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        seed ^= std::hash<T2>{}(p.second) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        return seed;
    }
};

//enumerators to contain all Team(Black,White,...) and Piece_Type(Pawn,Horse,Queen,...)
enum Team {
    White,
    Black,
    Null,
    Ghost,
    Target,
    WhiteSelected,
    BlackSelected,

};
enum Piece_Type {
    None,
    Pawn,
    King,
    Queen,
    Bishop,
    Horse,
    Castle
};
inline const char* TeamToString(Team t) {
    switch (t) {
    case White: return "White";
    case Black: return "Black";
    case WhiteSelected: return "WhiteSelected";
    case BlackSelected: return "BlackSelected";
    case Null: return "Null";
    default: return "None";
    }
}
inline const char* PieceTypeToString(Piece_Type t) {
    switch (t) {
    case Pawn: return "Pawn";
    case Castle: return "Castle";
    case Horse: return "Horse";
    case Bishop: return "Bishop";
    case Queen: return "Queen";
    case King: return "King";
    case None: return "Null";
    default: return "None";
    }
}

class Game_Piece : public VulkanHelper { // Added 'public' inheritance specifier.  
private:
    ModelLoader* Model;
    TextureImage* Texture;
    Team team = Null;
    Piece_Type type;


    ModelLoader hitboxModel;

    glm::mat4 localTransform = glm::mat4(1.0f);  // relative to chessboard
    glm::mat4 worldTransform = glm::mat4(1.0f);  // world space

public:
    //constructors
    Game_Piece() = default;
    Game_Piece(ModelLoader* model, TextureImage* texture, Team team, Piece_Type type)
        : Model(model), Texture(texture), team(team), type(type) {
    }

    //getters and setters
    glm::mat4 getLocalTransform() {
        return this->localTransform;
    }
    glm::mat4 getWorldTransform() {
        return this->worldTransform;
    }
    glm::vec3 getWorldPosition() const {
        return glm::vec3(worldTransform[3]); // Translation part of the matrix
    }
    void setLocalTransform(glm::mat4 t) {
        localTransform = t;
    }
    void setWorldTransform(glm::mat4 t) {
        worldTransform = t;
    }
    void setTeam(Team t) {
        team = t;
    }
    void setType(Piece_Type t) {
        this->type = t;
    }
    void setTexture(TextureImage* ti) {
        Texture = ti;
    }
    void setHitboxModel(const ModelLoader& model) {
        hitboxModel = model;
    }
    Team getTeam() const {
        return team;
    }
    Piece_Type getType() const {
        return type;
    }
    std::string getTeamString() const {
        return TeamToString(team);
    }
    std::string getTypeString() const {
        return PieceTypeToString(type);
    }
    ModelLoader& getHitboxModel() {
        return hitboxModel;
    }
    ModelLoader* getModel() {
        return this->Model;
    }
    TextureImage* getTexture() {
        return this->Texture;
    }
    void setModel(ModelLoader* model) {
        this->Model = model;
    }

    //change local transform to world transform
    void updateWorldTransform(const glm::mat4& parent) {
        worldTransform = parent * localTransform;
    }

    //reset all variables to defaults(empty space on board)
    void selfDestruct() {
        team = Team::Null;
        type = Piece_Type(); // default value
        Model = nullptr;
        Texture = nullptr;
        localTransform = glm::mat4(1.0f);
        worldTransform = glm::mat4(1.0f);
    }

};
class move {
public:
    Team team;
    Piece_Type pieceType;
    glm::vec3 origionalPosition;
    glm::vec3 newPosition;
    Piece_Type take_piece;

    move(Team t, Piece_Type pt, glm::vec3 og, glm::vec3 newP, Piece_Type take_piece)
        : team(t), pieceType(pt), origionalPosition(og), newPosition(newP), take_piece(take_piece) {
    }
    move() {
        team = Null;
        pieceType = None;
        origionalPosition = { -1,-1,-1 };
        newPosition = { -1,-1,-1 };
        take_piece = None;
    }
    bool operator==(const move& other) const {
        return pieceType == other.pieceType &&
            origionalPosition == other.origionalPosition &&
            newPosition == other.newPosition &&
            take_piece == other.take_piece &&
            team == other.team;
    }

};
// Chess_Logic.h
class Chess_Logic {
public:
    static std::vector<glm::vec3> PossibleMoves(Array3D<Game_Piece>& board, int x, int y, int z);
    static bool is_king_in_check(Array3D<Game_Piece>& board, Team team, glm::vec3& k);
    static bool is_checkMate(Array3D<Game_Piece>& board, Team team,glm::vec3& k);
    static std::vector<glm::vec3> find_pieces(Array3D<Game_Piece>& board, Team team, const std::vector<Piece_Type>& types);
    static std::vector<glm::vec3> getLegalMoves(Array3D<Game_Piece>& board, int x, int y, int z, Team team, glm::vec3& k);
    static void simulateMove(Array3D<Game_Piece>& board, move m);
    static inline Team getOppositeTeam(Team team) {
        return (team == White) ? Black : White;
    }
    static bool is_square_attacked(Array3D<Game_Piece>& board,const glm::vec3& target,Team attackerTeam);
    static bool is_staleMate(Array3D<Game_Piece>& board, Team team, glm::vec3 k);
};
