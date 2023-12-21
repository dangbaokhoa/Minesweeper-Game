#include <stdlib.h>
#include <time.h>
#include <bits/stdc++.h>
#include "raylib.h"

const int screenHeight = 1200;
const int screenWidth = 1200;
const int edgeLength = 35;
using namespace std;

const char* youlose = "YOU LOSE!";
const char* youwin = "YOU WIN!";
const char* pressRToRestart = "Press 'r' to play again";
const char* pressXToBack = "Press 'x' to go back to main menu";

// Information
int mines = 5;
int rows = 12, columns = 12;
int tileRevealed = 0;
bool firstTimeClick = false;
int clicked;
bool playMode = 0;
Rectangle dirArrow;
// Score
int score;
//
bool typingText[3] = {0, 0};
char name[3][10] = {{"\0"}, {"\0"}};
int letterCount[3] = {0, 0};

int lastTime = 0;
int marginTop = (screenHeight - rows * edgeLength) / 2;
int marginLeft = (screenWidth - columns * edgeLength) / 2;
enum menuState {MENU, PLAY, OPTION, HIGHSCORE, EXIT, SAVE, BACK}; 
typedef enum GameState {
    PLAYING,
    LOSE,
    WIN
} GameState;
menuState buttonState;

typedef struct Cell {
    int r;
    int c;
    bool containsMine;
    bool revealed;
    bool flagged;
    int nearbyMines;
    bool mark;
} Cell;
GameState state;
Cell grid[100][100];

Texture2D flagSprite;

int timeGameStarted;
int timeGameEnded;

void CellDraw(Cell);
bool IndexIsValid(int, int);
void CellReveal(int, int);
void PlaceMine(Cell); 
void GridInit(void);
void CalculateCell();
void GridFloodClearFrom(int, int);
void GameInit();
void DrawButton(const char [], int);
void PlayWorkspace();
void OptionWorkspace();
void DimensionOption(int, const char []);
void HighScoreWorkspace();
void CalculateScore();
void MarkGrid();
void ScoreUpdate();
void VariableFileIn();
void VariableFileOut();
int STOI(const char []);
void CellRevealed(int, int);
int main() {
    srand(time(0));
    InitWindow(screenWidth, screenHeight, "Raylib Template");

    flagSprite = LoadTexture("resources/flag.png");

    GameInit();
    VariableFileIn();

    SetTargetFPS(60);
    
    while (!WindowShouldClose()) {
        BeginDrawing();

            ClearBackground(RAYWHITE);
            DrawText(TextFormat("Row: %i Column: %i Mine: %i", rows, columns, mines), screenWidth - 400, 10, 25, GRAY);
            if (buttonState == MENU) {
                DrawButton("PLAY", 230);
                DrawButton("OPTION", 230 + 80);
                DrawButton("HIGH SCORE", 230 + 80 * 2);
                DrawButton("EXIT", 230 + 80 * 3);
            } else if (buttonState == PLAY) {
                PlayWorkspace();
                VariableFileOut();
            } else if (buttonState == OPTION) {
                OptionWorkspace();
            } else if (buttonState == HIGHSCORE) {
                HighScoreWorkspace();
            } else if (buttonState == EXIT) {
                break;
            } else if (buttonState == SAVE) {
                rows = STOI(name[0]);
                columns = STOI(name[1]);
                mines = STOI(name[2]);
                while (mines >= rows * columns) mines = rand() % (rows * columns);
                lastTime = 0;
                VariableFileOut();
                GameInit();
                buttonState = MENU;
            } else if (buttonState == BACK) {
                buttonState = MENU;
            }
            if (IsKeyPressed(KEY_X)) {
                VariableFileOut();
                buttonState = MENU;
            }
        EndDrawing();
    }
    lastTime = round(timeGameEnded - timeGameStarted) + lastTime;
    cout << lastTime << endl;
    VariableFileOut();
    CloseWindow();
    return 0;
}

int STOI(const char text[]) {
    int num = 0;
    for (int i = 0; text[i] != ' ' && text[i] != '_' && text[i] != '\0' && i < 10; i++) {
        num = num * 10 + (int) text[i] - 48;
    }
    return num;
}

void DrawButton(const char text[], int y) {
    Rectangle textBox = {(screenWidth - 400) * 0.5f, y * 1.0f, 400, 50};
    bool mouseOnText = false;

    if (CheckCollisionPointRec(GetMousePosition(), textBox)) mouseOnText = true;
    else mouseOnText = false;

    if (mouseOnText) DrawRectangleRec(textBox, BLACK);
    else DrawRectangleRec(textBox, WHITE);

    if (mouseOnText) DrawRectangleLines((int)textBox.x, (int)textBox.y, (int)textBox.width, (int)textBox.height, RED);
    else DrawRectangleLines((int)textBox.x, (int)textBox.y, (int)textBox.width, (int)textBox.height, BLACK);

    if (mouseOnText) DrawText(text, (int) textBox.x + (textBox.width - MeasureText(text, 35)) / 2, (int) textBox.y + (textBox.height - 35) / 2, 35, MAROON);
    else DrawText(text, (int) textBox.x + (textBox.width - MeasureText(text, 35)) / 2, (int) textBox.y + (textBox.height - 35) / 2, 35, BLACK);

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        if (CheckCollisionPointRec(GetMousePosition(), textBox)) {
           if (text == "PLAY") buttonState = PLAY;
           else if (text == "OPTION") buttonState = OPTION;
           else if (text == "HIGH SCORE") buttonState = HIGHSCORE;
           else if (text == "EXIT") buttonState = EXIT;
           else if (text == "SAVE") buttonState = SAVE;
           else if (text == "BACK") buttonState = BACK;
        }
    }
}

void ScoreUpdate(void) {
    fstream file;
    file.open("src/score_file.txt", ios::in);
    int highScore = 0;
    double highScoreTime = 0.0;
    double eff = 0;
    file >> highScore >> highScoreTime >> eff;
    file.close();
    if (highScore < score) highScore = score;
    double gameTime = timeGameEnded - timeGameStarted + lastTime;
    if (gameTime > 0) {
        if (highScoreTime < double(score) / gameTime) highScoreTime = double(score) / gameTime;
    }
    if (clicked > 0) {
        if (eff < double(score) / double(clicked)) eff = double(score) / double(clicked);
    }
    file.open("src/score_file.txt", ios::out);
    file << highScore << " " << highScoreTime << " " << eff;
    file.close();
}

void HighScoreWorkspace() {
    fstream file;
    file.open("src/score_file.txt", ios::in);
    int highScore = 0;
    double highScoreTime = 0.0;
    double eff = 0;
    file >> highScore >> highScoreTime >> eff;
    file.close();
    DrawText(TextFormat("3BV: %i", highScore), 250, 200, 40, BLACK);
    DrawText(TextFormat("3BV/s: %f", highScoreTime), 250, 250, 40, BLACK);
    DrawText(TextFormat("Efficiency: %f", eff), 250, 300, 40, BLACK);
    DrawButton("BACK", 650);
}

void DimensionOption(int type, const char text[], int y) {
    Rectangle textBox = {(screenWidth - 200) * 0.5f, y * 1.0f, 200, 50};
    bool mouseOnText = false;
    if (CheckCollisionPointRec(GetMousePosition(), textBox)) {
        mouseOnText = true;
    } 
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        if (CheckCollisionPointRec(GetMousePosition(), textBox)) {
           typingText[type] = true; 
        }  else typingText[type] = false;
    }

    if (typingText[type]) {
        int key = GetCharPressed();

        while (key > 0) {
            if ((key >= 32) && (key <= 125)) {
                name[type][letterCount[type]] = (char) key;
                name[type][letterCount[type] + 1] = '\0';
                letterCount[type]++;
            }
            key = GetCharPressed();
        }
        if (IsKeyPressed(KEY_BACKSPACE)) {
            name[type][letterCount[type]] = '\0';
            letterCount[type]--;
            if (letterCount[type] < 0) letterCount[type] = 0;
            name[type][letterCount[type]] = '\0';
        }  
    }
    DrawText(text, (int)textBox.x, (int) textBox.y - 50, 40, GRAY); 
    if (mouseOnText || typingText[type]) {
        name[type][letterCount[type]] = '_';
    } else name[type][letterCount[type]] = '\0';
    DrawRectangleRec(textBox, WHITE);
    if (mouseOnText) SetMouseCursor(MOUSE_CURSOR_IBEAM);
    else SetMouseCursor(MOUSE_CURSOR_DEFAULT);
    DrawRectangleLines((int)textBox.x, (int)textBox.y, (int)textBox.width, (int)textBox.height, DARKGRAY);
    DrawText(name[type], (int)textBox.x + 5, (int)textBox.y + 8, 40, BLACK);
}
void OptionWorkspace() {
    DimensionOption(0, "ROWS", 200);
    DimensionOption(1, "COLUMNS", 350);
    DimensionOption(2, "MINES", 500);
    DrawText("If number of mine > rows * columns it will randomize", 150, 590, 20, GRAY);
    DrawButton("SAVE", 650);
}

void MarkGrid(int c, int r) {
    if (grid[c][r].mark) return ;
    grid[c][r].mark = 1;
    for (int i = -1; i <= 1; i++) {
        for (int j = -1; j <= 1; j++) {
            if (!i && !j) continue;
            if (IndexIsValid(c + i, r + j)) {
                if (grid[c + i][r + j].mark != 0) MarkGrid(c + i, r + j);
            }
        }
    }
}

void CalculateScore() {
    for (int c = 0; c < columns; c++) {
        for (int r = 0; r < rows; r++) {
            if (grid[c][r].nearbyMines == 0 && grid[c][r].containsMine != 0 && grid[c][r].mark == 0) {
                MarkGrid(c, r);
                score++;
            }
        }
    }

    for (int c = 0; c < columns; c++) {
        for (int r = 0; r < rows; r++) {
            if (grid[c][r].nearbyMines != 0 && grid[c][r].containsMine != 0 && grid[c][r].mark != 0) {
                score++;
            }
        }
    }
}
void PlayWorkspace() {
    int gameTime = timeGameEnded - timeGameStarted + lastTime;
    
    if (state == PLAYING) {
        timeGameEnded = GetTime();
        DrawText(TextFormat("Time: %i minutes %i second", (int) gameTime / 60, (int) gameTime % 60), 10, 10, 25, BLACK);
        DrawText(TextFormat("Press t to play by %s", (!playMode ? "arrow key" : "mouse")), 10, 40, 25, GRAY);
        DrawText(TextFormat(pressXToBack), 10, screenHeight - 50, 25, GRAY);
        int indexC = (GetMousePosition().x - marginLeft) / edgeLength;
        int indexR = (GetMousePosition().y - marginTop) / edgeLength;
        if (IndexIsValid(indexC, indexR) && (GetMousePosition().x - marginLeft > 0) && (GetMousePosition().y - marginTop > 0)) {
            DrawRectangle(indexC * edgeLength + marginLeft, indexR * edgeLength + marginTop, edgeLength, edgeLength, BLUE);
        }
    }
    if (IsKeyPressed(KEY_T)) {
        playMode ^= 1;
        dirArrow.x = marginLeft + 13;
        dirArrow.y = marginTop + 13;
        dirArrow.width = dirArrow.height = 15;
    }
    if (IsKeyPressed(KEY_R)) {
        lastTime = 0;
        score = 0;
        GameInit();
        VariableFileOut();
    }

    for (int c = 0; c < columns; c++) {
        for (int r = 0; r < rows; r++) {
            CellDraw(grid[c][r]);
        }
    }
    if (state == LOSE) {
        score = 0;
        DrawRectangle(0, 0, screenWidth, screenHeight, Fade(WHITE, 0.8f));
        DrawText(youlose, screenWidth / 2 - MeasureText(youlose, 20) / 2, screenHeight / 2 - 10, 20, DARKGRAY);
        DrawText(pressRToRestart, screenWidth / 2 - MeasureText(pressRToRestart, 20) / 2, screenHeight * 0.75 - 10, 20, DARKGRAY);
        DrawText(pressXToBack, screenWidth / 2 - MeasureText(pressXToBack, 20) / 2, screenHeight * 0.75 + 20, 20, DARKGRAY);
        DrawText(TextFormat("Time: %i minutes %i second", (int) gameTime / 60, (int) gameTime % 60), screenWidth / 2 - 120, screenHeight * 0.75 + 50, 20, DARKGRAY);
        for (int i = 0; i < rows; i++) {
            for (int j = 0; j < columns; j++) {
                if (grid[i][j].containsMine) grid[i][j].revealed = 1;
            }
        }
        return;
    } else if (state == WIN) {
        ScoreUpdate();
        DrawRectangle(0, 0, screenWidth, screenHeight, Fade(WHITE, 0.8f));
        DrawText(youwin, screenWidth / 2 - MeasureText(youlose, 20) / 2, screenHeight / 2 - 10, 20, DARKGRAY);
        DrawText(pressRToRestart, screenWidth / 2 - MeasureText(pressRToRestart, 20) / 2, screenHeight * 0.75 - 10, 20, DARKGRAY);
        DrawText(pressXToBack, screenWidth / 2 - MeasureText(pressXToBack, 20) / 2, screenHeight * 0.75 + 20, 20, DARKGRAY);
        DrawText(TextFormat("Time: %i minutes %i second", (int) gameTime / 60, (int) gameTime % 60), screenWidth / 2 - 120, screenHeight * 0.75 + 50, 20, DARKGRAY);
        return;
    }
    if (playMode == 0) {
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            Vector2 mPos = GetMousePosition();
            int indexC = (mPos.x - marginLeft) / edgeLength;
            int indexR = (mPos.y - marginTop) / edgeLength;
            if (IndexIsValid(indexC, indexR) && (mPos.x - marginLeft > 0) && (mPos.y - marginTop > 0)) {
                if (!firstTimeClick) {
                    firstTimeClick = true;
                    PlaceMine(grid[indexC][indexR]);
                    CalculateCell();
                    CalculateScore();
                }
                if (!grid[indexC][indexR].revealed) clicked++;
                else CellRevealed(indexC, indexR);
                CellReveal(indexC, indexR);
            }
        } 

        if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
            Vector2 mPos = GetMousePosition();
            int indexC = (mPos.x - marginLeft) / edgeLength;
            int indexR = (mPos.y - marginTop) / edgeLength;
            if (IndexIsValid(indexC, indexR) && (mPos.x - marginLeft > 0) && (mPos.y - marginTop > 0)) {
                grid[indexC][indexR].flagged ^= 1;
            }
        }
    } else {
        DrawText(TextFormat("Press shift to flag"), 10, 80, 25, GRAY);
        if (IsKeyReleased(KEY_RIGHT)) if (IndexIsValid((dirArrow.x + edgeLength - marginLeft) / edgeLength, (dirArrow.y - marginTop) / edgeLength)) dirArrow.x += edgeLength;
        if (IsKeyReleased(KEY_LEFT)) if (IndexIsValid((dirArrow.x - edgeLength - marginLeft) / edgeLength, (dirArrow.y - marginTop) / edgeLength)) dirArrow.x -= edgeLength;
        if (IsKeyReleased(KEY_UP)) if (IndexIsValid((dirArrow.x - marginLeft) / edgeLength, (dirArrow.y - marginTop - edgeLength) / edgeLength)) dirArrow.y -= edgeLength;
        if (IsKeyReleased(KEY_DOWN)) if (IndexIsValid((dirArrow.x - marginLeft) / edgeLength, (dirArrow.y - marginTop + edgeLength) / edgeLength)) dirArrow.y += edgeLength;
        DrawRectangleRec(dirArrow, MAROON);
        if (IsKeyReleased(KEY_ENTER)) {
            int indexC = (dirArrow.x - marginLeft) / edgeLength;
            int indexR = (dirArrow.y - marginTop) / edgeLength;
            if (IndexIsValid(indexC, indexR) && (dirArrow.x - marginLeft) > 0 && (dirArrow.y - marginTop) > 0) {
                if (!firstTimeClick) {
                    firstTimeClick = true;
                    PlaceMine(grid[indexC][indexR]);
                    CalculateCell();
                    CalculateScore();
                }
                if (!grid[indexC][indexR].revealed) clicked++;
                else CellRevealed(indexC, indexR);
                CellReveal(indexC, indexR);
            }
        }
         if (IsKeyReleased(KEY_RIGHT_SHIFT) || IsKeyReleased(KEY_LEFT_SHIFT)) {
            int indexC = (dirArrow.x - marginLeft) / edgeLength;
            int indexR = (dirArrow.y - marginTop) / edgeLength;
            if (IndexIsValid(indexC, indexR) && (dirArrow.x - marginLeft) > 0 && (dirArrow.y - marginTop) > 0) {
                grid[indexC][indexR].flagged ^= 1;
            }
         }
    }
}
void CalculateCell() { 
    for (int c = 0; c < columns; c++) {
        for (int r = 0; r < rows; r++) {
            int count = 0;
            for (int i = -1; i <= 1; i++) {
                for (int j = -1; j <= 1; j++) {
                    if (!i && !j) continue;
                    if (!IndexIsValid(c + i, r + j)) continue;
                    if (grid[c + i][r + j].containsMine) {
                        count++;
                    }
                }
            }
            grid[c][r].nearbyMines = count;
        }
    }
}

void PlaceMine(Cell cell) { // Set mine in the cell instead of cell [r, c]
    for (int c = 0; c < columns; c++) {
        for (int r = 0; r < rows; r++) {
            cell.containsMine = false;
        }
    }
    
    int minesLeft = mines;
    while (minesLeft > 0) {
        int c = rand() % columns;
        int r = rand() % rows;
        if (r == cell.r && c == cell.c) continue;
        if (grid[c][r].containsMine == false) {
            grid[c][r].containsMine = true;
            minesLeft--;
        }
    }
}

void CellDraw(Cell cell) {
    if (cell.revealed) {
        if (cell.containsMine) {
            DrawRectangle(cell.c * edgeLength + marginLeft, cell.r * edgeLength + marginTop, edgeLength, edgeLength, RED);
        } else {
            DrawRectangle(cell.c * edgeLength + marginLeft, cell.r * edgeLength + marginTop, edgeLength, edgeLength, LIGHTGRAY);
            if (cell.nearbyMines > 0) {
                DrawText(TextFormat("%d", cell.nearbyMines), cell.c * edgeLength + marginLeft + 10, cell.r * edgeLength + marginTop + 3, edgeLength, DARKGRAY);
            }
        }
    } else if (cell.flagged) {
        Rectangle source = {0, 0, flagSprite.width * 1.0f, flagSprite.height * 1.0f};
        Rectangle dest = {cell.c * edgeLength + marginLeft * 1.0f, (int) cell.r * edgeLength + marginTop * 1.0f, edgeLength, edgeLength};
        Vector2 origin = {0, 0};  

        DrawTexturePro(flagSprite, source, dest, origin, 0.0f, Fade(WHITE, 0.8f));
    }

    DrawRectangleLines(cell.c * edgeLength + marginLeft, cell.r * edgeLength + marginTop, edgeLength, edgeLength, BLACK);
}

bool IndexIsValid(int c, int r) {
	return c >= 0 && c < columns && r >= 0 && r < rows;
}

void GridFloodClearFrom(int c, int r) {
    for (int i = -1; i <= 1; i++) {
        for (int j = -1; j <= 1; j++) {
            if (!i && !j) continue;
            if (!IndexIsValid(c + i, r + j)) continue;
            CellReveal(c + i, r + j);
        }
    }
}

void CellReveal(int c, int r) {
    if (grid[c][r].flagged) return;
    if (grid[c][r].revealed) return ;

    grid[c][r].revealed = 1;
    tileRevealed++;

    if (grid[c][r].containsMine) {
        state = LOSE;
        return ;
    } else if (grid[c][r].nearbyMines == 0) {   
        GridFloodClearFrom(c, r);
    }

    if (tileRevealed == rows * columns - mines) {
        state = WIN;
    }
}
void VariableFileOut() {
    ofstream file;
    file.open("src/old_game_session.txt", ios::out);
    file << columns << " " << rows << " " << mines;
    for (int c = 0; c < columns; c++) {
        for (int r = 0; r < rows; r++) {
            Cell &cell = grid[c][r];
            file << " " << cell.r << " " << cell.c << " " << cell.nearbyMines << " " << cell.revealed << " " << cell.flagged << " " << cell.nearbyMines << " " << cell.mark;
        }
    }
    file << " " << tileRevealed;
    file << " " << firstTimeClick;
    if (state == PLAYING) {
        file << " " << 0;
    } else if (state == LOSE) {
        file << " " << 1;
    } else file << " " << 2;
    file << " " << clicked;
    file << " " << marginTop;
    file << " " << marginLeft;
    file << " " << lastTime;
    file.close();
}

void VariableFileIn() {
    ifstream file;
    file.open("src/old_game_session.txt", ios::in);
    file >> columns >> rows >> mines;
    for (int c = 0; c < columns; c++) {
        for (int r = 0; r < rows; r++) {
            Cell &cell = grid[c][r];
            file >> cell.r >> cell.c >> cell.nearbyMines >> cell.revealed >> cell.flagged >> cell.nearbyMines >> cell.mark;
        }
    }
    file >> tileRevealed;
    file >> firstTimeClick;
    int st;
    file >> st;
    if (st == 0) {
        state = PLAYING;
    } else if (st == 1) {
        state = LOSE;
    } else state = WIN;
    file >> clicked;
    file >> marginTop;
    file >> marginLeft;
    file >> lastTime;
    file.close();
}

void GridInit(void) {
    for (int c = 0; c < columns; c++) {
        for (int r = 0; r < rows; r++) {
           grid[c][r] = (Cell) {
            .r = r, 
            .c = c,
            .containsMine = false,
            .revealed = false,
            .flagged = false,
            .nearbyMines = 0,
            .mark = 0
            };
        }
    }
}

void CellRevealed(int c, int r) {
    int flagCount = 0;
    for (int i = -1; i <= 1; i++) {
        for (int j = -1; j <= 1; j++) {
            if (!i && !j) continue;
            if (!IndexIsValid(c + i, r + j)) continue;
            flagCount += grid[c + i][r + j].flagged;
        }
    }
    if (flagCount == grid[c][r].nearbyMines) {
        for (int i = -1; i <= 1; i++) {
            for (int j = -1; j <= 1; j++) {
                if (!i && !j) continue;
                if (!IndexIsValid(c + i, r + j)) continue;
                if (grid[c + i][r + j].flagged) continue;
                if (grid[c + i][r + j].containsMine) {
                    state = LOSE;
                    return ;
                } else {
                    if (!grid[c + i][r + j].revealed == 1) tileRevealed++;
                    grid[c + i][r + j].revealed = 1;
                    if (tileRevealed == rows * columns - mines) {
                        state = WIN;
                    }
                }
            }
        }
    }
}

void GameInit() {
    GridInit();
    ScoreUpdate();
    tileRevealed = 0;
    firstTimeClick = 0;
    state = PLAYING;
    clicked = 0;
    timeGameStarted = GetTime();
    marginTop = (screenHeight - rows * edgeLength) / 2;
    marginLeft = (screenWidth - columns * edgeLength) / 2;
}