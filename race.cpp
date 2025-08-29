#include <iostream>
#include <string>
#include <random>
#include <chrono>
#include <thread>
#include <fstream>
#include <limits>
#include <cctype>

using namespace std;

static inline string toUpperFirst(const string& s) {
    if (s.empty()) return "";
    string t = s;
    t[0] = static_cast<char>(toupper(static_cast<unsigned char>(t[0])));
    return t;
}

int main() {
    // --- Input ---
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
    if (cin >> frameDelaySeconds) {
        if (frameDelaySeconds < 0) frameDelaySeconds = 0;
    } else {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        frameDelaySeconds = 0.25;
    }

    unsigned int seedInput = 0;
    cout << "Seed (0 = random): ";
    if (!(cin >> seedInput)) {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        seedInput = 0;
    }

    char colorChoice = 'y';
    cout << "Use colours? (y/n): ";
    cin >> colorChoice;
    bool useColours = (colorChoice == 'y' || colorChoice == 'Y');

    // --- Randomness ---
    unsigned int seed = (seedInput == 0)
        ? static_cast<unsigned int>(chrono::high_resolution_clock::now().time_since_epoch().count())
        : seedInput;
    mt19937 randomEngine(seed);
    uniform_int_distribution<int> baseStep(1, 6);
    uniform_int_distribution<int> nitroChance(1, 8); // 1-in-8 chance per tick for +3

    // --- Race state ---
    int racerOnePosition = 0;
    int racerTwoPosition = 0;
    bool raceHasFinished = false;
    int ticks = 0;

    // racer symbols (initials). If same letter, use 1 and 2
    char racerOneSymbol = racerOneName.empty() ? '1' : static_cast<char>(toupper(static_cast<unsigned char>(racerOneName[0])));
    char racerTwoSymbol = racerTwoName.empty() ? '2' : static_cast<char>(toupper(static_cast<unsigned char>(racerTwoName[0])));
    if (racerOneSymbol == racerTwoSymbol) { racerOneSymbol = '1'; racerTwoSymbol = '2'; }

    // viewport settings
    const int viewportWidthMin = 40;
    int viewportWidth = max(viewportWidthMin, min(finishLinePosition + 1, 80)); // 40..80 chars visible

    auto clearScreen = []() {
        cout << "\033[2J\033[1;1H";
    };
    auto colour = [&](const string& s, const string& code) -> string {
        if (!useColours) return s;
        return "\033[" + code + "m" + s + "\033[0m";
    };

    auto drawLane = [&](const string& name, char symbol, int position, int viewStart, int viewEnd) {
        int visibleLen = viewEnd - viewStart + 1;
        string lane(visibleLen, '-');

        // clamp symbol position into viewport
        int pos = position;
        if (pos < viewStart) pos = viewStart;
        if (pos > viewEnd)   pos = viewEnd;

        // place racer
        lane[pos - viewStart] = symbol;

        // mark finish line if inside viewport (draw a '|')
        if (finishLinePosition >= viewStart && finishLinePosition <= viewEnd) {
            lane[finishLinePosition - viewStart] = '|';
        }

        cout << colour(name, "1;37") << ": " << lane << "\n";
    };

    // --- Countdown ---
    clearScreen();
    cout << "=== ASCII Race Track ===\n";
    cout << racerOneName << " (" << racerOneSymbol << ") vs "
         << racerTwoName << " (" << racerTwoSymbol << ") • First to position " << finishLinePosition
         << " wins!\n";
    cout << "Seed: " << seed << "   (viewport auto-scrolls)\n\n";
    cout << "3...\n"; this_thread::sleep_for(chrono::milliseconds(400));
    cout << "2...\n"; this_thread::sleep_for(chrono::milliseconds(400));
    cout << "1...\n"; this_thread::sleep_for(chrono::milliseconds(400));
    cout << "GO!\n";   this_thread::sleep_for(chrono::milliseconds(400));

    // --- Main race loop ---
    while (!raceHasFinished) {
        ++ticks;

        // base random step
        int step1 = baseStep(randomEngine);
        int step2 = baseStep(randomEngine);

        // occasional nitro (+3)
        if (nitroChance(randomEngine) == 1) step1 += 3;
        if (nitroChance(randomEngine) == 1) step2 += 3;

        racerOnePosition += step1;
        racerTwoPosition += step2;

        if (racerOnePosition > finishLinePosition) racerOnePosition = finishLinePosition;
        if (racerTwoPosition > finishLinePosition) racerTwoPosition = finishLinePosition;

        // viewport scroll: center on the furthest racer
        int lead = max(racerOnePosition, racerTwoPosition);
        int viewStart = max(0, lead - viewportWidth + 10); // keep some space ahead
        int viewEnd   = min(finishLinePosition, viewStart + viewportWidth - 1);
        if (viewEnd - viewStart + 1 < viewportWidth) {
            viewStart = max(0, viewEnd - viewportWidth + 1);
        }

        // redraw
        clearScreen();
        cout << "=== ASCII Race Track ===\n";
        cout << "Finish line: " << finishLinePosition << " | Tick: " << ticks << " | Seed: " << seed << "\n\n";

        drawLane(racerOneName, racerOneSymbol, racerOnePosition, viewStart, viewEnd);
        drawLane(racerTwoName, racerTwoSymbol, racerTwoPosition, viewStart, viewEnd);
        cout << "\n";

        // Commentary
        if (racerOnePosition > racerTwoPosition) {
            cout << "Commentary: " << colour(racerOneName, "32") << " takes the lead!\n";
        } else if (racerTwoPosition > racerOnePosition) {
            cout << "Commentary: " << colour(racerTwoName, "36") << " surges ahead!\n";
        } else {
            cout << "Commentary: Neck and neck!\n";
        }

        // finish checks
        bool oneAtFinish = (racerOnePosition >= finishLinePosition);
        bool twoAtFinish = (racerTwoPosition >= finishLinePosition);

        if (oneAtFinish && twoAtFinish) {
            cout << "\nResult: It's a TIE! 🏁\n";
            raceHasFinished = true;
        } else if (oneAtFinish) {
            cout << "\nResult: " << colour(racerOneName, "32;1") << " wins! 🏆\n";
            raceHasFinished = true;
            ofstream lb("race_leaderboard.txt", ios::app);
            if (lb) lb << racerOneName << " beat " << racerTwoName << " in " << ticks << " ticks (seed " << seed << ")\n";
        } else if (twoAtFinish) {
            cout << "\nResult: " << colour(racerTwoName, "36;1") << " wins! 🏆\n";
            raceHasFinished = true;
            ofstream lb("race_leaderboard.txt", ios::app);
            if (lb) lb << racerTwoName << " beat " << racerOneName << " in " << ticks << " ticks (seed " << seed << ")\n";
        }

        if (!raceHasFinished) {
            this_thread::sleep_for(chrono::milliseconds(static_cast<int>(frameDelaySeconds * 1000)));
        }
    }

    cout << "\nThanks for racing!\n";
    return 0;
}
