#include <iostream>
#include <string>
#include <vector>
#include <map>

using namespace std;

struct Account{
    string id;
    int balance;
};

struct Card{
    string cardnumber;
};

enum class ATMState{
    IDLE, CARD_INSERTED, AUTHENTICATED, ACCOUNT_SELECTED
};

struct cardMeta{
    int pin;
    bool locked = false;
    int pinFailCount = 0;
    vector<Account> accounts;
};

map<string, cardMeta> bankDB = {
    {"1234-5678", {1234, false, 0, {{"A001", 100}, {"A002",  200}}}},
    {"9999-0000", {4321, false, 0, {{"MAIN", 50}}}}
};


Card* g_currentCard = nullptr;
Account* g_currentAccount = nullptr;
ATMState g_state = ATMState::IDLE;

const int MAX_PIN_FAIL = 3;

bool requireCard(){
    if(!g_currentCard){
        cout << "카드가 삽입되지 않았습니다." << endl;
        return false;
    }
    return true;
}
bool requireAuth(){
    if(g_state != ATMState::AUTHENTICATED && g_state != ATMState::ACCOUNT_SELECTED){
        cout << "PIN 인증 필요합니다." << endl;
        return false;
    }
    return true;
}
bool requireAccount(){
    if(g_state != ATMState::ACCOUNT_SELECTED || !g_currentAccount){
        cout << "계좌가 선택되지 않았습니다." << endl;
        return false;
    }
    return true;
}

bool insertCard(Card* card){
    if(g_state != ATMState::IDLE){
        cout << "카드 인식 중 입니다." << endl;
        return false;
    }
    if(!card){
        cout << "잘못된 카드 입니다." << endl;
        return false;
    }
    auto it = bankDB.find(card->cardnumber);
    if(it == bankDB.end()){
        cout << "등록되지 않은 카드" << endl;
        return false;
    }
    if(it->second.locked){
        cout << "해당 카드는 잠긴 카드입니다." << endl;
        return false;
    }
    g_currentAccount = nullptr;
    g_currentCard = card;
    g_state = ATMState::CARD_INSERTED;
    cout << "카드 삽입됨: " << card->cardnumber << endl;
    return true;
}

bool enterPIN(int pin){
    if(g_state != ATMState::CARD_INSERTED){
        cout << "PIN 입력 단계가 아닙니다." << endl;
        return false;
    }
    if(!requireCard())return false;

    auto &meta = bankDB[g_currentCard->cardnumber];
    if(meta.locked){
        cout << "잠긴 카드 입니다." << endl;
        return false;
    }
    if(meta.pin == pin){
        meta.pinFailCount=0;
        g_state = ATMState::AUTHENTICATED;
        cout << "PIN인증 성공" << endl;
        return true;
    }
    else{
        meta.pinFailCount++;
        cout << "PIN 인증 실패(실패횟수: " << meta.pinFailCount <<")"<< endl;
        if(meta.pinFailCount >= MAX_PIN_FAIL){
            meta.locked = true;
            cout << "PIN 인증 실패로 카드 잠김" << endl;
        }
        return false;
    }
}

vector<Account*> getAccountPointers(){
    vector<Account*> out;
    if(!requireAuth()) return out;
    auto &meta = bankDB[g_currentCard->cardnumber];
    for(auto &acc : meta.accounts){
        out.push_back(&acc);
    }
    return out;
}

void printAccounts(){
    auto list = getAccountPointers();
    if(list.empty())return;
    cout << "계좌목록" << endl;
    for(auto *acc : list){
        cout << "-" << acc->id << " 잔액: "<< acc->balance << endl;
    }
}

bool selectAccount(const string& accountId){
    if(!requireAuth()) return false;
    auto list = getAccountPointers();
    for(auto *acc : list){
        if(acc->id == accountId){
            g_currentAccount = acc;
            g_state = ATMState::ACCOUNT_SELECTED;
            cout << "선택된 계좌: "<< accountId << endl;
            return true;
        }
    }
    cout << "존재하지 않는 계좌"<< endl;
    return false;
}

int getBalance(){
    if(!requireAccount()) return -1;
    return g_currentAccount->balance;
}
bool deposit(int amount){
    if(!requireAccount())return false;
    if(amount <= 0){
        cout << "입금 금액 오류" << endl;
        return false;
    }
    g_currentAccount->balance += amount;
    cout << "입금: " << amount << " | 현재 잔액: " << g_currentAccount->balance << endl;
    return true;
}
bool withdraw(int amount){
    if(!requireAccount()) return false;
    if(amount <= 0){
        cout << "출금 금액 오류" << endl;
        return false;
    }
    if(g_currentAccount->balance < amount){
        cout << "잔액부족 (요청: " << amount << " | 잔액" << g_currentAccount->balance << ")" <<endl;
        return false;
    }
    g_currentAccount->balance -= amount;
    cout << "출금: "<< amount << " | 잔액" << g_currentAccount->balance<<endl;
    return true;
}
bool ejectCard(){
    if(!g_currentCard){
        cout << "반환할 카드 없음"<<endl;
        return false;
    }
    cout << "카드 반환" << g_currentCard->cardnumber<<endl;
    g_currentCard = nullptr;
    g_currentAccount = nullptr;
    g_state = ATMState::IDLE;
    return true;
}
void resetAll(){
    g_currentAccount = nullptr;
    g_currentCard = nullptr;
    g_state = ATMState::IDLE;
    for(auto &kv : bankDB){
        kv.second.pinFailCount =0;
    }
}
//테스트 시나리오 1: 정상흐름
void testScenarioSuccess(){
    cout << "===Test1: 정상흐름, 입금===" << endl;
    resetAll();
    Card c{"1234-5678"};
    insertCard(&c);
    enterPIN(1234);
    printAccounts();
    selectAccount("A001");
    cout << "초기잔액: "<<getBalance()<<endl;
    deposit(10);
    cout << "초기잔액: "<<getBalance()<<endl;
    ejectCard();
}
//테스트 시나리오2: PIN 실패 및 잠금
void testScenarioPinFail(){
    cout << "===Test2: PIN 실패 및 잠금===" << endl;
    resetAll();
    Card c{"9999-0000"};
    insertCard(&c);
    enterPIN(1111);
    enterPIN(2222);
    enterPIN(3333);//PIN번호 3회 오류
    enterPIN(4321);//lock 되었으므로 올바른 PIN번호도 실패
    ejectCard();
}
//테스트 시나리오3: 잔액 부족으로 인한 출금 실패
void testScenarioWithdrawFail(){
    cout << "===Test3: 잔액부족으로 인한 출금 실패===" << endl;
    resetAll();
    Card c{"9999-0000"};
    insertCard(&c);
    enterPIN(4321);
    selectAccount("MAIN");
    cout << "현재 잔액: " << getBalance() << endl;
    withdraw(999);
    withdraw(10);
    cout << "현재 잔액: " << getBalance() << endl;
    ejectCard();
}



int main(){
    cout << "====ATM컨트롤러===="<<endl;
    testScenarioSuccess();
    testScenarioPinFail();
    testScenarioWithdrawFail();
    return 0;
}
