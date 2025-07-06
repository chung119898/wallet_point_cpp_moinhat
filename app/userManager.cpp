# Giải thích chi tiết từng dòng mã trong `app/userManager.cpp`

> **Lưu ý**: File này định nghĩa các chức năng quản lý người dùng, đăng ký/đăng nhập, xác thực OTP, quản lý ví, chuyển điểm, đổi mật khẩu v.v.  
> Các chú thích sẽ bám sát từng dòng/từng khối lệnh.

---

```c++
#include "userManager.h"                // Bao gồm file header định nghĩa lớp UserManager và các phụ thuộc
#include <cstdlib>                      // Thư viện cho system("cls") để xóa màn hình console

#define SECRET_KEY "JBSWY3DPEHPK3PXP"   // Định nghĩa khóa bí mật dùng cho OTP (bạn có thể thay đổi khi triển khai)
```
- Bao gồm các file header cần thiết.  
- Định nghĩa một SECRET_KEY dùng cho tạo mã OTP.

---

```c++
// ==================== Khởi tạo UserManager ====================
UserManager::UserManager() {
    userDatabase = make_unique<UserDatabase>();            // Khởi tạo CSDL người dùng (dùng smart pointer)
    walletManager = make_unique<WalletManager>();          // Khởi tạo quản lý ví
    transactionManager = make_unique<TransactionManager>(); // Khởi tạo quản lý giao dịch
    userDatabase->backupDatabase("data/backup/users.db");  // Tự động backup dữ liệu user khi khởi động
}
```
- **UserManager::UserManager()**: Constructor khởi tạo 3 đối tượng quản lý: user, ví, giao dịch.  
- `make_unique` tạo smart pointer, tự động giải phóng sau khi dùng.  
- Khi khởi động sẽ tự động backup dữ liệu user.

---

```c++
UserManager::~UserManager() {
    // Không cần xóa thủ công vì dùng smart pointer
}
```
- Destructor, không cần dọn dẹp thủ công vì đã dùng smart pointer.

---

```c++
// ==================== Sinh mật khẩu ngẫu nhiên ====================
string UserManager::generatePassword() {
    const string chars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789"; // Các ký tự hợp lệ
    const int length = 8;                                    // Độ dài mật khẩu

    random_device rd;                                        // Tạo seed ngẫu nhiên từ hệ thống
    mt19937 generator(rd());                                 // Khởi tạo bộ sinh số ngẫu nhiên
    uniform_int_distribution<int> dist(0, chars.size() - 1); // Tạo phân phối đều trong khoảng ký tự

    string password;                                         // Chuỗi mật khẩu kết quả
    for (int i = 0; i < length; ++i) {                      // Lặp 8 lần
        password += chars[dist(generator)];                  // Lấy ngẫu nhiên 1 ký tự và nối vào password
    }

    return password;                                         // Trả về mật khẩu đã tạo
}
```
- Hàm sinh mật khẩu ngẫu nhiên gồm 8 ký tự (chữ hoa, thường, số).  
- Sử dụng random_device và mt19937 để đảm bảo tính ngẫu nhiên mạnh.

---

```c++
// ==================== Sinh mã OTP (One-time password) ====================
void UserManager::generateOTP() {
    char otp[7] = { 0 };                                    // Khởi tạo mảng 6 ký tự + 1 ký tự null kết thúc
    OTPData data;                                           // Cấu trúc dữ liệu OTP
    totp_new(&data, SECRET_KEY, hmac_algo_sha1, getCurrentTime, 6, 30); // Khởi tạo cấu hình OTP: 6 số, 30s, SHA1
    totp_now(&data, otp);                                   // Sinh mã OTP hiện tại dựa trên thời gian
    cout << "Ma OTP cua ban la: " << otp << endl;           // Hiển thị OTP ra màn hình
    getCurrentTime();                                       // (Có thể chỉ để cập nhật state thời gian)
}
```
- Tạo OTP 6 số, mỗi 30 giây đổi mã, thuật toán SHA1.
- In mã OTP ra màn hình (thường dùng để xác thực hành động nhạy cảm).

---

```c++
// ==================== Kiểm tra mã OTP nhập vào có đúng không ====================
bool UserManager::verifyOTP(const string& userOTP) {
    char otp[7] = { 0 };                                    // Sinh lại mã OTP hiện tại (phải trùng với lúc gửi)
    OTPData data;
    totp_new(&data, SECRET_KEY, hmac_algo_sha1, getCurrentTime, 6, 30); // Config OTP như trên
    totp_now(&data, otp);                                   // Sinh mã OTP hiện tại
    cout << "Xin moi nhap OTP: " << userOTP << endl;        // In OTP người dùng vừa nhập (debug)
    getCurrentTime();                                       // (Có thể chỉ để cập nhật state thời gian)
    return (userOTP == otp);                                // So sánh mã nhập với mã hệ thống
}
```
- Sinh lại OTP hiện tại rồi so sánh với mã người dùng nhập.  
- Đúng thì trả về true, sai thì false.

---

```c++
// ==================== Đăng ký tài khoản người dùng thường ====================
void UserManager::registerUser() {
    system("cls");                                          // Xóa màn hình console
    string username, password, fullName, phoneNumber;       // Khai báo biến thông tin user
    int amount = 1000;                                      // Số điểm mặc định cấp cho user mới

    cout << "\nDANG KY NGUOI DUNG" << endl;                 // In tiêu đề
    cout << "Nhap ten nguoi dung: ";
    cin >> username;                                        // Nhập username

    if (userDatabase->userExists(username)) {               // Kiểm tra trùng tên
        cout << "Loi: Ten nguoi dung da ton tai!" << endl;
        return;                                             // Thoát nếu đã tồn tại
    }

    cout << "Nhap mat khau: ";
    cin >> password;                                        // Nhập mật khẩu

    cout << "Nhap ho va ten: ";
    cin.ignore(numeric_limits<streamsize>::max(), '\n');    // Xóa bộ nhớ đệm trước khi dùng getline
    getline(cin, fullName);                                 // Nhập họ tên (có thể chứa dấu cách)

    regex phonePattern("^0\\d{9}$");                        // Regex kiểm tra số điện thoại (10 số, bắt đầu bằng 0)
    do {
        cout << "Nhap so dien thoai: ";
        getline(cin, phoneNumber);                          // Nhập số điện thoại
        if (!regex_match(phoneNumber, phonePattern))        // Nếu không hợp lệ
            cout << "So dien thoai khong dung dinh dang! Vui long nhap lai." << endl;
        else
            break;                                          // Hợp lệ thì thoát vòng lặp
    } while (true);

    userDatabase->addUser(username, password, "user", fullName, phoneNumber, 0); // Thêm user vào CSDL

    if (!walletManager->deductFromMaster(amount)) {         // Trừ điểm từ ví master (ví tổng)
        cout << "Loi: So du vi tong khong du de cap diem cho nguoi dung moi." << endl;
        return;
    }

    string walletId = walletManager->createWallet(username, amount); // Tạo ví mới cho user

    if (!walletId.empty()) {                                // Nếu tạo ví thành công
        cout << "Dang ky thanh cong! Ban co ngay 1000 diem!" << endl;
    }
    else {                                                  // Nếu tạo ví thất bại
        cout << "Dang ky thanh cong! Nhung co loi khi tao vi." << endl;
    }
}
```
- Đăng ký tài khoản mới: nhập thông tin, kiểm tra username trùng, kiểm tra số điện thoại hợp lệ, tạo user, cấp điểm và ví cho user.

---

```c++
// ==================== Quản trị viên tạo tài khoản hộ ====================
void UserManager::registerUserForOthers() {
    system("cls");                                          // Xóa màn hình
    string username, password = generatePassword(), fullName, phoneNumber; // Tự sinh mật khẩu
    int amount = 1000;                                      // Số điểm tặng cho user mới

    cout << "\nNHAN VIEN QUAN LY - TAO TAI KHOAN HO" << endl;
    cout << "Nhap ten nguoi dung moi: ";
    cin >> username;

    if (userDatabase->userExists(username)) {               // Kiểm tra trùng tên
        cout << "Loi: Ten nguoi dung da ton tai!" << endl;
        return;
    }

    cout << "Nhap ho va ten: ";
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    getline(cin, fullName);

    regex phonePattern("^0\\d{9}$");                        // Regex kiểm tra số điện thoại
    do {
        cout << "Nhap so dien thoai: ";
        getline(cin, phoneNumber);
        if (!regex_match(phoneNumber, phonePattern))
            cout << "So dien thoai khong dung dinh dang! Vui long nhap lai." << endl;
        else
            break;
    } while (true);

    userDatabase->addUser(username, password, "user", fullName, phoneNumber, 1); // Thêm user (bắt buộc đổi mật khẩu lần đầu)

    if (!walletManager->deductFromMaster(amount)) {         // Trừ điểm ví tổng
        cout << "Loi: So du vi tong khong du de cap diem cho nguoi dung moi." << endl;
        return;
    }

    string walletId = walletManager->createWallet(username, amount); // Tạo ví cho user

    if (!walletId.empty()) {
        cout << "Ban da duoc cap 1000 diem. Wallet ID: " << walletId << endl;
    }
    else {
        cout << "Dang ky thanh cong! Nhung co loi khi tao vi." << endl;
    }

    cout << "Tao tai khoan thanh cong!\nMat khau: " << password << " (Bat buoc doi sau khi dang nhap)" << endl;
}
```
- Chức năng cho quản trị viên tạo hộ tài khoản cho người khác, mật khẩu tự sinh, user bắt buộc đổi sau khi đăng nhập lần đầu.

---

```c++
// ==================== Đăng nhập tài khoản ====================
User UserManager::loginUser() {
    string username, password;                              // Biến tạm lưu thông tin đăng nhập
    cout << "Nhap ten nguoi dung: ";
    cin >> username;

    cout << "Nhap mat khau: ";
    cin >> password;

    if (!userDatabase->userExists(username)) {              // Kiểm tra user tồn tại
        cout << "Nguoi dung khong ton tai!" << endl;
        return User{};                                      // Trả về user rỗng nếu không tồn tại
    }

    User user = userDatabase->getUser(username);            // Lấy thông tin user

    if (!userDatabase->verifyPassword(password, user.password)) { // Kiểm tra mật khẩu
        cout << "Mat khau sai!" << endl;
        return User{};
    }

    if (user.mustChangePassword) {                          // Nếu bắt buộc đổi mật khẩu
        cout << "Ban can doi mat khau ngay lap tuc!" << endl;
        changePassword(username);                            // Yêu cầu đổi mật khẩu
        return User{};
    }

    // Nếu có thay đổi thông tin đang chờ xác nhận (pending)
    if (userDatabase->hasPendingChange(username)) {
        system("cls");
        auto [pendingFullName, pendingPhone] = userDatabase->getPendingChange(username); // Lấy thông tin chờ xác nhận
        cout << "Tai khoan cua ban co thay doi chua duoc xac nhan!" << endl;
        cout << "Thong tin thay doi:" << endl;
        cout << "Ho ten moi: " << pendingFullName << endl;
        cout << "So dien thoai moi: " << pendingPhone << endl;
        cout << "----------------------------" << endl;

        generateOTP();                                      // Gửi mã OTP xác nhận thay đổi
        string pendingOTP;
        cout << "\nNhap ma OTP de xac nhan thay doi (Canh bao: sai OTP thi thong tin se khong duoc cap nhat): ";
        cin >> pendingOTP;
        if (verifyOTP(pendingOTP)) {                        // Nếu OTP đúng
            userDatabase->confirmPendingChange(username);    // Xác nhận thay đổi
            cout << "Thay doi da duoc xac nhan!" << endl;
            user = userDatabase->getUser(username);          // Cập nhật lại user
        }
        else {                                              // Nếu OTP sai
            cout << "Ma OTP khong dung! Thay doi bi huy. Vui long thu lai sau." << endl;
            userDatabase->rejectPendingChange(username);     // Hủy thay đổi
            return User{};
        }
    }

    cout << "Dang nhap thanh cong!" << endl;

    if (user.role == "manager") {                           // Nếu là quản trị viên
        showManagerMenu(username);                          // Vào menu quản lý
    }
    else {
        showUserMenu(username);                             // Vào menu người dùng thường
    }

    return user;                                            // Trả về thông tin user sau khi login thành công
}
```
- Tiến trình đăng nhập:  
  + Nhập username, password  
  + Kiểm tra tồn tại, kiểm tra đúng mật khẩu  
  + Nếu bắt buộc đổi pass => yêu cầu đổi  
  + Nếu có thông tin pending thay đổi thì xác thực OTP  
  + Đăng nhập thành công sẽ chuyển vào menu phù hợp theo vai trò.

---

```c++
// ==================== Chuyển điểm giữa các ví ====================
void UserManager::transferFunds(const string& senderUsername) {
    string receiverWalletId;                                // ID ví nhận điểm
    int amount;                                             // Số điểm chuyển
    cout << "Nhap vi nguoi nhan diem: ";
    cin >> receiverWalletId;
    cout << "Nhap so luong diem: ";
    cin >> amount;

    generateOTP();                                          // Gửi mã OTP xác nhận giao dịch
    string userOTP;
    cout << "Xin moi nhap ma OTP de xac nhan giao dich: ";
    cin >> userOTP;

    if (!verifyOTP(userOTP)) {                              // Nếu OTP sai
        cout << "OTP xac thuc khong thanh cong. Vui long thuc hien lai giao dich!" << endl;
        return;
    }

    string senderWalletId = walletManager->getWalletIdByUsername(senderUsername); // Lấy ID ví gửi
    if (senderWalletId.empty()) {                           // Nếu không tìm thấy ví
        cout << "Khong tim thay thong tin vi!" << endl;
    }
    else {
        // Thực hiện chuyển điểm
        bool success = walletManager->transferPoints(senderWalletId, receiverWalletId, amount);
        string senderDisplay = userDatabase->getUser(senderUsername).fullName; // Lấy tên người gửi
        string receiverUsername = walletManager->getUsernameByWalletId(receiverWalletId); // Lấy username người nhận
        string receiverDisplay = receiverUsername.empty()
            ? "Unknown"
            : userDatabase->getUser(receiverUsername).fullName;        // Lấy tên người nhận hoặc Unknown

        cout << (success ? "Giao dich thanh cong!" : "Giao dich that bai!") << endl;
        transactionManager->recordTransaction(
            senderWalletId, senderDisplay,
            receiverWalletId, receiverDisplay,
            amount, "Chuyen diem",
            success ? "Thanh cong" : "That bai"
        ); // Ghi nhận lịch sử giao dịch
    }
    cout << "Press any key to continue..." << endl;         // Đợi người dùng xác nhận trước khi quay lại menu
    cin.ignore();
    cin.get();
}
```
- Chuyển điểm giữa các ví: nhập ví nhận, số điểm, xác thực OTP, thực hiện chuyển, ghi nhận lịch sử giao dịch.

---

```c++
// ==================== Đổi mật khẩu tài khoản ====================
void UserManager::changePassword(const string& username) {
    string newPassword, userOTP;

    cout << "Nhap mat khau moi: ";
    cin >> newPassword;                                     // Nhập mật khẩu mới

    generateOTP();                                          // Gửi OTP xác nhận đổi mật khẩu
    cout << "Nhap ma OTP de xac nhan: ";
    cin >> userOTP;

    if (verifyOTP(userOTP)) {                               // Nếu OTP đúng
        userDatabase->updateUserPassword(username, newPassword); // Cập nhật mật khẩu mới vào CSDL
        userDatabase->updateMustChangePassword(username, 0); // Bỏ cờ bắt buộc đổi mật khẩu nếu có
        cout << "Doi mat khau thanh cong!" << endl;
    }
    else {                                                  // Nếu OTP sai
        cout << "Ma OTP khong dung! Huy doi mat khau." << endl;
    }
}
```
- Đổi mật khẩu: nhập pass mới, xác thực OTP, cập nhật vào CSDL.

---

```c++
// ==================== Lấy thông tin user từ CSDL ====================
bool UserManager::loadUserInfo(const string& username, User& user) {
    if (!userDatabase->userExists(username)) {              // Kiểm tra user tồn tại
        return false;
    }
    user = userDatabase->getUser(username);                 // Lấy thông tin user ra biến tham chiếu
    return true;
}
```
- Lấy thông tin user từ CSDL, trả về true nếu tồn tại.

---

```c++
// ==================== Cập nhật thông tin người dùng ====================
void UserManager::updateUserInfo(const string& currentUsername, User& currentUser) {
    string targetUsername;
    if (currentUser.role == "manager") {                    // Nếu là quản lý
        cout << "Nhap ten nguoi dung can cap nhat (nhap '0' de cap nhat thong tin cua ban): ";
        cin >> targetUsername;
        if (targetUsername == "0") {                        // Nếu nhập 0, cập nhật cho chính mình
            targetUsername = currentUsername;
        }
    }
    else {
        targetUsername = currentUsername;                   // Người dùng thường chỉ được cập nhật chính mình
    }

    User targetUser = userDatabase->getUser(targetUsername); // Lấy thông tin user cần cập nhật
    if (targetUser.username.empty()) {                      // Nếu user không tồn tại
        cout << "Nguoi dung khong ton tai!" << endl;
        return;
    }

    string newFullName, newPhoneNumber, userOTP;

    cout << "Nhap ho ten moi (nhap '0' neu khong cap nhat): ";
    getline(cin >> ws, newFullName);                        // Nhập họ tên mới

    if (newFullName == "0") {                               // Nếu nhập 0, giữ họ tên cũ
        newFullName = targetUser.fullName;
    }

    cout << "Nhap so dien thoai moi (nhap '0' neu khong cap nhat): ";
    getline(cin, newPhoneNumber);                           // Nhập SĐT mới

    if (newPhoneNumber == "0") {                            // Nếu nhập 0, giữ SĐT cũ
        newPhoneNumber = targetUser.phoneNumber;
    }
    else if (!newPhoneNumber.empty()) {                     // Nếu nhập mới, kiểm tra định dạng
        regex phonePattern("^0\\d{9}$");
        while (!regex_match(newPhoneNumber, phonePattern)) {
            cout << "So dien thoai khong dung dinh dang! Vui long nhap lai (hoac nhan Enter de bo qua): ";
            getline(cin, newPhoneNumber);
            if (newPhoneNumber.empty())                     // Nếu bỏ qua, thoát lặp
                break;
        }
    }

    generateOTP();                                          // Gửi OTP xác nhận cập nhật thông tin
    cout << "Nhap ma OTP de xac nhan: ";
    cin >> userOTP;

    if (verifyOTP(userOTP)) {                               // Nếu OTP đúng
        if (currentUser.role == "manager" && targetUsername != currentUsername) {
            userDatabase->addPendingChange(targetUsername, newFullName, newPhoneNumber); // Nếu quản lý cập nhật hộ, thông tin sẽ chờ xác nhận
            cout << "Cap nhat thong tin thanh cong! Thay doi se co hieu luc sau khi xac nhan." << endl;
        }
        else {
            userDatabase->updateUserInfo(targetUsername, newFullName, newPhoneNumber);   // Người dùng tự cập nhật hoặc quản lý tự cập nhật cho mình
            cout << "Cap nhat thong tin thanh cong!" << endl;
        }
    }
    else {
        cout << "Ma OTP khong dung! Huy cap nhat." << endl; // Nếu OTP sai
    }
}
```
- Cập nhật thông tin user.  
- Nếu là quản lý có thể cập nhật hộ người khác (bắt buộc xác nhận bằng OTP).  
- Nếu là user thường chỉ cập nhật được thông tin của chính mình.

---

```c++
// ==================== Menu dành cho quản trị viên ====================
void UserManager::showManagerMenu(const string& username) {
    User manager = userDatabase->getUser(username);          // Lấy thông tin quản lý

    if (manager.role != "manager") {                        // Chỉ cho phép menu này nếu là quản lý
        return;
    }

    int choice;
    do {
        int balance = walletManager->getBalance("000000");  // Lấy số dư ví tổng
        cout << "\n=== MENU QUAN LY ===" << endl;
        cout << "Ho va ten: " << manager.fullName << endl;
        cout << "So dien thoai: " << manager.phoneNumber << endl;
        cout << "So du vi tong: " << balance << " diem" << endl;
        cout << "Vai tro: Quan tri vien" << endl;
        cout << "----------------------------" << endl;
        cout << "1. Tao tai khoan ho" << endl;
        cout << "2. Doi mat khau" << endl;
        cout << "3. Cap nhat thong tin nguoi dung" << endl;
        cout << "4. Dang xuat" << endl;
        cout << "Chon: ";
        cin >> choice;

        switch (choice) {
        case 1:
            registerUserForOthers();                        // Tạo tài khoản hộ người khác
            break;
        case 2:
            changePassword(manager.username);               // Đổi mật khẩu cho chính mình
            manager = userDatabase->getUser(username);      // Cập nhật lại thông tin quản lý
            break;
        case 3:
            updateUserInfo(username, manager);              // Cập nhật thông tin user bất kỳ
            manager = userDatabase->getUser(username);      // Cập nhật lại thông tin quản lý
            break;
        case 4:
            cout << "Dang xuat thanh cong!" << endl;        // Thoát menu quản lý
            system("cls");
            return;
        default:
            cout << "Lua chon khong hop le!" << endl;       // Lựa chọn sai
        }
    } while (choice != 4);                                  // Lặp đến khi chọn đăng xuất
}
```
- Menu dành cho quản lý: tạo account hộ, đổi pass, cập nhật thông tin user bất kỳ, đăng xuất.

---

```c++
// ==================== Menu dành cho người dùng thường ====================
void UserManager::showUserMenu(const string& username) {
    User user = userDatabase->getUser(username);            // Lấy thông tin user
    int choice;
    do {
        system("cls");                                      // Xóa màn hình
        int balance = walletManager->getBalanceByUsername(username); // Lấy số dư ví
        string walletid = walletManager->getWalletIdByUsername(username); // Lấy ID ví
        cout << "\n=== MENU NGUOI DUNG ===" << endl;
        cout << "Ho va ten: " << user.fullName << endl;
        cout << "So dien thoai: " << user.phoneNumber << endl;
        cout << "So tai khoan: " << walletid  << endl;
        cout << "So du: " << balance << " diem" << endl;
        cout << "Vai tro: " << user.role << endl;
        cout << "----------------------------" << endl;
        cout << "1. Doi mat khau" << endl;
        cout << "2. Thay doi thong tin" << endl;
        cout << "3. Xem lich su giao dich" << endl;
        cout << "4. Chuyen diem" << endl;
        cout << "5. Dang xuat" << endl;
        cout << "Chon: ";
        cin >> choice;

        if (cin.fail()) {                                   // Nếu nhập sai kiểu dữ liệu (ví dụ nhập chữ)
            cin.clear();                                    // Xóa lỗi nhập
            cin.ignore(numeric_limits<streamsize>::max(), '\n'); // Xóa bộ nhớ đệm
            cout << "Lua chon khong hop le! Vui long nhap lai." << endl;
            continue;
        }

        switch (choice) {
        case 1:
            changePassword(user.username);                  // Đổi mật khẩu
            user = userDatabase->getUser(username);         // Cập nhật lại thông tin user
            break;
        case 2:
            updateUserInfo(username, user);                 // Cập nhật thông tin bản thân
            user = userDatabase->getUser(username);         // Cập nhật lại thông tin user
            break;
        case 3: {
            string walletId = walletManager->getWalletIdByUsername(username); // Lấy ID ví
            vector<string> history = transactionManager->getTransactionHistory(walletId); // Lấy lịch sử giao dịch
            cout << "\n--- Lich su giao dich ---" << endl;
            for (const auto& record : history) {            // Lặp qua từng giao dịch
                cout << record << endl;
            }
            cout << "Nhan phim bat ky de tiep tuc..." << endl;
            cin.ignore(); cin.get();                        // Chờ người dùng nhấn Enter
            break;
        }
        case 4:
            transferFunds(user.username);                   // Chuyển điểm cho người khác
            break;
        case 5:
            cout << "Dang xuat thanh cong!" << endl;        // Thoát menu
            system("cls");
            return;
        default:
            cout << "Lua chon khong hop le!" << endl;       // Lựa chọn sai
        }
    } while (choice != 5);                                  // Lặp đến khi chọn đăng xuất
}
```
- Menu dành cho user thường: đổi pass, cập nhật thông tin, xem lịch sử giao dịch, chuyển điểm, đăng xuất.

---

**Tóm lại:**  
File này tổ chức đầy đủ nghiệp vụ quản lý user, xác thực OTP, giao dịch, cập nhật thông tin, phân quyền quản trị viên/người dùng thường, có xác thực OTP cho các hành động nhạy cảm.
