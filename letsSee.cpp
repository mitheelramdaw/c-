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
#include <algorithm>

using namespace std;


static inline string toUpperFirst(const string& s) {
    if (s.empty()) return "";
    string t = s;
    t[0] = static_cast<char>(toupper(static_cast<unsigned char>(t[0])));
    return t;
}

int main() {
    int finishLinePosition;
    cout << "Enter the race track length (minimum 20): ";
    cin >> finishLinePosition;
    if (!cin || finishLinePosition < 20) {
        cerr << "Read you monkey MORE THAN 20.\n";
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
    cout << "How often do you want updates (seconds, (e.g.) 0.25): ";
    if (!(cin >> frameDelaySeconds) || frameDelaySeconds < 0) {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        frameDelaySeconds = 0.25;
    }

    unsigned int seedInput = 0;
    cout << "Race Code (0 = random): ";
    if (!(cin >> seedInput)) {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        seedInput = 0;
    }

    char colorChoice = 'y';
    cout << "Use colours? (y/n): ";
    cin >> colorChoice;
    bool useColours = (colorChoice == 'y' || colorChoice == 'Y');

    string theme;
    cout << "Track theme ('space', 'snow', 'desert', 'city', 'jungle'): ";
    cin.ignore();
    getline(cin, theme);
    char themeChar = '-';
    if (theme == "space") themeChar = '@';
    else if (theme == "snow") themeChar = '*';
    else if (theme == "desert") themeChar = '~';
    else if (theme == "city") themeChar = '#';
    else if (theme == "jungle") themeChar = '+';

    unsigned int seed = (seedInput == 0)
        ? static_cast<unsigned int>(chrono::high_resolution_clock::now().time_since_epoch().count())
        : seedInput;
    mt19937 randomEngine(seed);
    uniform_int_distribution<int> baseStep(1, 6);
    uniform_int_distribution<int> nitroChance(1, 8);
    uniform_int_distribution<int> commentDist(0, 3);

    unordered_map<int, string> specialTiles;
    vector<int> candidatePositions;
    for (int i = 5; i < finishLinePosition - 1; i += 3) candidatePositions.push_back(i);
    shuffle(candidatePositions.begin(), candidatePositions.end(), randomEngine);
    int numTiles = min((int)candidatePositions.size(), max(2, finishLinePosition / 5));
    for (int i = 0; i < numTiles; ++i) {
        specialTiles[candidatePositions[i]] = (i % 2 == 0) ? "powerup" : "hazard";
    }

    int racerOnePosition = 0, racerTwoPosition = 0;
    int racerOneLaps = 0, racerTwoLaps = 0;
    const int maxLaps = 3;
    int turns = 0;
    bool raceHasFinished = false;

    char racerOneSymbol = toupper(racerOneName[0]);
    char racerTwoSymbol = toupper(racerTwoName[0]);
    if (racerOneSymbol == racerTwoSymbol) {
        racerOneSymbol = '1';
        racerTwoSymbol = '2';
    }

    const int viewportWidthMin = 40;
    int viewportWidth = max(viewportWidthMin, min(finishLinePosition + 1, 80));

    auto clearScreen = []() {
        cout << "\033[2J\033[1;1H";
    };
    auto colour = [&](const string& s, const string& code) {
        return useColours ? "\033[" + code + "m" + s + "\033[0m" : s;
    };

    vector<string> leadComments = {
        "is pulling ahead!",
        "takes control of the track!",
        "is leading the race!",
        "surges forward! Pile first"
    };
    vector<string> tieComments = {
        "It's bunds and bunds!",
        "Both racers dead even!",
        "No separation at all!",
        "They’re locked together!"
    };

    auto applyTileEffect = [&](int& pos, const string& name) {
        if (specialTiles.count(pos)) {
            if (specialTiles[pos] == "powerup") {
                cout << colour(" >> BOOST << ", "1;33") << " " << colour(name, "33") << " smashed a power-up! +3!\n";
                pos += 3;
            } else if (specialTiles[pos] == "hazard") {
                cout << colour(" >> HAZARD << ", "1;31") << " " << colour(name, "31") << " mbonzied a hazard! -2!\n";
                pos = max(0, pos - 2);
            }
        }
    };

    auto drawLane = [&](const string& name, char symbol, int pos, int viewStart, int viewEnd, int laps) {
        int len = viewEnd - viewStart + 1;
        vector<string> lane(len, string(1, themeChar));
        for (int i = 0; i < len; ++i) {
            int p = viewStart + i;
            if (specialTiles.count(p)) {
                if (specialTiles[p] == "powerup") lane[i] = colour("P", "1;33");
                else if (specialTiles[p] == "hazard") lane[i] = colour("H", "1;31");
            }
        }
        if (pos >= viewStart && pos <= viewEnd) {
            string symbolStr(1, symbol);
            string symbolColour = (symbol == racerOneSymbol) ? "1;32" : "1;36";  // Green or Cyan
            lane[pos - viewStart] = colour(symbolStr, symbolColour);
        }
        if (finishLinePosition >= viewStart && finishLinePosition <= viewEnd)
            lane[finishLinePosition - viewStart] = "|";

        cout << colour(name, "1;37") << ": ";
        for (auto& c : lane) cout << c;
        cout << " [Lap " << laps << "/" << maxLaps << "]\n";
    };

    // Countdown
    clearScreen();
    cout << "=== ASCII Race Track ===\n";
    cout << racerOneName << " (" << racerOneSymbol << ") VS  "
         << racerTwoName << " (" << racerTwoSymbol << ") • First to " << maxLaps << " laps wins!\n";
    cout << "Track Length: " << finishLinePosition << " | Race Code: " << seed << "\n\n";
    cout << colour("3...", "1;31") << endl; this_thread::sleep_for(chrono::milliseconds(400));
    cout << colour("2...", "1;33") << endl; this_thread::sleep_for(chrono::milliseconds(400));
    cout << colour("1...", "1;32") << endl; this_thread::sleep_for(chrono::milliseconds(400));
    cout << colour("GO!", "1;36") << endl;   this_thread::sleep_for(chrono::milliseconds(400));

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
        cout << "Laps: First to " << maxLaps << " | Turn: " << turns << " | Race Code: " << seed << "\n\n";

        drawLane(racerOneName, racerOneSymbol, racerOnePosition, viewStart, viewEnd, racerOneLaps);
        drawLane(racerTwoName, racerTwoSymbol, racerTwoPosition, viewStart, viewEnd, racerTwoLaps);
        cout << "\n" << colour("=== COMMENTARY ===", "1;44") << "\n";

        if (racerOnePosition > racerTwoPosition) {
            cout << colour(racerOneName, "1;32") << " " << leadComments[commentDist(randomEngine)] << "\n";
        } else if (racerTwoPosition > racerOnePosition) {
            cout << colour(racerTwoName, "1;36") << " " << leadComments[commentDist(randomEngine)] << "\n";
        } else {
            cout << colour(tieComments[commentDist(randomEngine)], "1;35") << "\n";
        }

        if (racerOneLaps >= maxLaps && racerTwoLaps >= maxLaps) {
            cout << "\nResult: It's a TIE! 🏁\n";
            raceHasFinished = true;
        } else if (racerOneLaps >= maxLaps) {
            cout << "\nResult: " << colour(racerOneName, "32;1") << " wins! 🏆\n";
            raceHasFinished = true;
        } else if (racerTwoLaps >= maxLaps) {
            cout << "\nResult: " << colour(racerTwoName, "36;1") << " wins! 🏆\n";
            raceHasFinished = true;
        }

        if (!raceHasFinished)
            this_thread::sleep_for(chrono::milliseconds(static_cast<int>(frameDelaySeconds * 1000)));
    }

    cout << "\nGo home now the hell !\n";
    return 0;
}
