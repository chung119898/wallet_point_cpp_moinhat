1| ﻿#pragma once                             // Chỉ cho phép include file này một lần (tránh đụng độ định nghĩa)
2| 
3| #ifndef USERMANAGER_H                    // Nếu chưa định nghĩa USERMANAGER_H thì định nghĩa (bảo vệ include)
4| #define USERMANAGER_H
5| 
6| #include <iostream>                      // Thư viện nhập/xuất dữ liệu (cout, cin, ...)
7| #include <string>                        // Thư viện chuỗi C++
8| #include <regex>                         // Thư viện biểu thức chính quy (kiểm tra định dạng SĐT, ...)
9| #include <map>                           // Thư viện bản đồ ánh xạ (dùng trong các class phụ trợ khác)
10| #include <memory>                        // Thư viện smart pointer (unique_ptr, ...)
11| #include <cotp.h>                        // Thư viện mã OTP (One-time password)
12| 
13| #include "otp_utils.h"                   // Include hàm/định nghĩa hỗ trợ OTP (của dự án)
14| #include "userDatabase.h"                // Quản lý dữ liệu người dùng
15| #include "walletManager.h"               // Quản lý ví tiền/điểm
16| #include "transactionManager.h"          // Quản lý lịch sử giao dịch
17| #include "user.h"                        // Định nghĩa struct/class User
18| 
19| using namespace std;                     // Sử dụng toàn bộ không gian tên std để ngắn gọn cú pháp
20| 
21| class UserManager {                      // Định nghĩa lớp UserManager - quản lý toàn bộ nghiệp vụ người dùng
22| private:
23|     unique_ptr<UserDatabase> userDatabase;          // Con trỏ thông minh quản lý CSDL người dùng
24|     unique_ptr<WalletManager> walletManager;        // Con trỏ thông minh quản lý ví
25|     unique_ptr<TransactionManager> transactionManager; // Con trỏ thông minh quản lý giao dịch
26| 	string generatePassword();                       // Hàm tạo mật khẩu ngẫu nhiên (private)
27| 
28| public:
29|     UserManager();                                 // Hàm khởi tạo (constructor)
30|     ~UserManager();                                // Hàm hủy (destructor)
31| 
32|     bool verifyOTP(const string& userOTP);         // Kiểm tra mã OTP nhập vào có hợp lệ không
33|     void generateOTP();                            // Sinh và hiển thị mã OTP cho thao tác xác thực
34| 
35|     void registerUser();                           // Đăng ký tài khoản cho chính mình
36|     void registerUserForOthers();                  // Quản trị viên đăng ký tài khoản hộ người khác
37|     User loginUser();                              // Đăng nhập tài khoản
38|     bool loadUserInfo(const string& username, User& user); // Lấy thông tin chi tiết một user
39|     void transferFunds(const string& senderUsername);      // Chuyển điểm giữa các ví
40|     void showManagerMenu(const string& username);          // Hiển thị menu cho quản trị viên
41|     void showUserMenu(const string& user);                 // Hiển thị menu cho người dùng thường
42|     void changePassword(const string& username);           // Đổi mật khẩu tài khoản
43|     void updateUserInfo(const string& username, User& user); // Cập nhật thông tin user (họ tên, SĐT,...)
44| };
45| 
46| #endif                                  // Kết thúc khối bảo vệ include
47| 
