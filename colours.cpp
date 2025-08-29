#include <iostream>
#include <string>
#include <random>
#include <chrono>
#include <thread>
#include <fstream>
#include <limits>
#include <cctype>
#include <unordered_map>
#include <vector>

using namespace std;

static inline string toUpperFirst(const string& s) {
    if (s.empty()) return "";
    string t = s;
    t[0] = static_cast<char>(toupper(static_cast<unsigned char>(t[0])));
    return t;
}

int main() {
    // === User Input ===
    int finishLinePosition;
    cout << "Enter the race track length (minimum 20): ";
    cin >> finishLinePosition;
    if (!cin || finishLinePosition < 20) {
        cerr << "Read you monnkey MORE THAN 20.\n";
        return 1;
    }

    string racerOneName, racerTwoName;
    cout << "Enter name for Racer One: ";
    cin >> racerOneName;
    cout << "Enter name for Racer Two: ";
    cin >> racerTwoName;
    racerOneName = toUpperFirst(racerOneName);
    racerTwoName = toUpperFirst(racerTwoName);

    double frameDelaySeconds = 0.25;
    cout << "How often do you want updates (seconds, e.g., 0.25): ";
    if (!(cin >> frameDelaySeconds) || frameDelaySeconds < 0) {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        frameDelaySeconds = 0.25;
    }

    unsigned int raceCodeInput = 0;
    cout << "Enter Race Code (0 = random race): ";
    if (!(cin >> raceCodeInput)) {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        raceCodeInput = 0;
    }

    char colorChoice = 'y';
    cout << "Use colours? (y/n): ";
    cin >> colorChoice;
    bool useColours = (colorChoice == 'y' || colorChoice == 'Y');

    // === Random setup ===
    unsigned int raceCode = (raceCodeInput == 0)
        ? static_cast<unsigned int>(chrono::high_resolution_clock::now().time_since_epoch().count())
        : raceCodeInput;
    mt19937 randomEngine(raceCode);
    uniform_int_distribution<int> baseStep(1, 6);
    uniform_int_distribution<int> nitroChance(1, 8);
    uniform_int_distribution<int> commentDist(0, 3);

    // === Game state ===
    int racerOnePosition = 0, racerTwoPosition = 0;
    int racerOneLaps = 0, racerTwoLaps = 0;
    const int maxLaps = 3;
    int turns = 0;
    bool raceHasFinished = false;

    char racerOneSymbol = toupper(racerOneName[0]);
    char racerTwoSymbol = toupper(racerTwoName[0]);
    if (racerOneSymbol == racerTwoSymbol) { racerOneSymbol = '1'; racerTwoSymbol = '2'; }

    const int viewportWidthMin = 40;
    int viewportWidth = max(viewportWidthMin, min(finishLinePosition + 1, 80));

    auto clearScreen = []() {
        cout << "\033[2J\033[1;1H";
    };
    auto colour = [&](const string& s, const string& code) {
        return useColours ? "\033[" + code + "m" + s + "\033[0m" : s;
    };

    unordered_map<int, string> specialTiles = {
        {15, "powerup"}, {30, "hazard"}, {45, "powerup"}, {60, "hazard"}
    };

    vector<string> leadComments = {
        "is pulling ahead!",
        "takes control of the track!",
        "is leading the race!",
        "surges forward!"
    };
    vector<string> tieComments = {
        "It's neck and neck!",
        "Both racers dead even!",
        "No separation at all!",
        "They’re locked together!"
    };

    auto applyTileEffect = [&](int& pos, const string& name) {
        if (specialTiles.count(pos)) {
            if (specialTiles[pos] == "powerup") {
                cout << colour(name, "33") << " hits a power-up! +3 boost!\n";
                pos += 3;
            } else if (specialTiles[pos] == "hazard") {
                cout << colour(name, "31") << " hits a hazard! -2 penalty!\n";
                pos = max(0, pos - 2);
            }
        }
    };

    auto drawLane = [&](const string& name, char symbol, int pos, int viewStart, int viewEnd, int laps) {
        int len = viewEnd - viewStart + 1;
        string lane(len, '-');
        for (int i = 0; i < len; ++i) {
            int p = viewStart + i;
            if (specialTiles.count(p)) {
                lane[i] = (specialTiles[p] == "powerup") ? 'P' : 'H';
            }
        }
        if (pos >= viewStart && pos <= viewEnd)
            lane[pos - viewStart] = symbol;
        if (finishLinePosition >= viewStart && finishLinePosition <= viewEnd)
            lane[finishLinePosition - viewStart] = '|';

        cout << colour(name, "1;37") << ": " << lane << " [Lap " << laps << "/" << maxLaps << "]\n";
    };

    // === Countdown ===
    clearScreen();
    cout << "=== ASCII Race Track ===\n";
    cout << racerOneName << " (" << racerOneSymbol << ") vs "
         << racerTwoName << " (" << racerTwoSymbol << ") • First to " << maxLaps << " laps wins!\n";
    cout << "Track Length: " << finishLinePosition << " | Race Code: " << raceCode << "\n\n";

    cout << colour("3...", "31;1") << "\n"; this_thread::sleep_for(chrono::milliseconds(400));
    cout << colour("2...", "33;1") << "\n"; this_thread::sleep_for(chrono::milliseconds(400));
    cout << colour("1...", "32;1") << "\n"; this_thread::sleep_for(chrono::milliseconds(400));
    cout << colour("GO!",   "36;1") << "\n"; this_thread::sleep_for(chrono::milliseconds(400));

    // === Main game loop ===
    while (!raceHasFinished) {
        ++turns;

        int step1 = baseStep(randomEngine);
        int step2 = baseStep(randomEngine);
        if (nitroChance(randomEngine) == 1) step1 += 3;
        if (nitroChance(randomEngine) == 1) step2 += 3;

        racerOnePosition += step1;
        racerTwoPosition += step2;

        applyTileEffect(racerOnePosition, racerOneName);
        applyTileEffect(racerTwoPosition, racerTwoName);

        if (racerOnePosition >= finishLinePosition) {
            ++racerOneLaps;
            racerOnePosition = 0;
        }
        if (racerTwoPosition >= finishLinePosition) {
            ++racerTwoLaps;
            racerTwoPosition = 0;
        }

        int lead = max(racerOnePosition, racerTwoPosition);
        int viewStart = max(0, lead - viewportWidth + 10);
        int viewEnd = min(finishLinePosition, viewStart + viewportWidth - 1);
        if (viewEnd - viewStart + 1 < viewportWidth)
            viewStart = max(0, viewEnd - viewportWidth + 1);

        clearScreen();
        cout << "=== ASCII Race Track ===\n";
        cout << "Lap Target: " << maxLaps << " | Turn: " << turns << " | Race Code: " << raceCode << "\n\n";

        drawLane(racerOneName, racerOneSymbol, racerOnePosition, viewStart, viewEnd, racerOneLaps);
        drawLane(racerTwoName, racerTwoSymbol, racerTwoPosition, viewStart, viewEnd, racerTwoLaps);
        cout << "\n";

        if (racerOnePosition > racerTwoPosition) {
            cout << "Commentary: " << colour(racerOneName, "32") << " " << leadComments[commentDist(randomEngine)] << "\n";
        } else if (racerTwoPosition > racerOnePosition) {
            cout << "Commentary: " << colour(racerTwoName, "36") << " " << leadComments[commentDist(randomEngine)] << "\n";
        } else {
            cout << "Commentary: " << tieComments[commentDist(randomEngine)] << "\n";
        }

        if (racerOneLaps >= maxLaps && racerTwoLaps >= maxLaps) {
            cout << "\nResult: It's a TIE! 🏁\n";
            raceHasFinished = true;
        } else if (racerOneLaps >= maxLaps) {
            cout << "\nResult: " << colour(racerOneName, "32;1") << " wins! 🏆\n";
            raceHasFinished = true;
            ofstream lb("race_leaderboard.txt", ios::app);
            if (lb) lb << racerOneName << " beat " << racerTwoName << " in " << turns << " turns (Race Code: " << raceCode << ")\n";
        } else if (racerTwoLaps >= maxLaps) {
            cout << "\nResult: " << colour(racerTwoName, "36;1") << " wins! 🏆\n";
            raceHasFinished = true;
            ofstream lb("race_leaderboard.txt", ios::app);
            if (lb) lb << racerTwoName << " beat " << racerOneName << " in " << turns << " turns (Race Code: " << raceCode << ")\n";
        }

        if (!raceHasFinished)
            this_thread::sleep_for(chrono::milliseconds(static_cast<int>(frameDelaySeconds * 1000)));
    }

    cout << "\nThanks for racing!\n";
    return 0;
}
