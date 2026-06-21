# OPEP Qt GUI

Bản này dùng giao diện Qt polished, đã chỉnh lại theo yêu cầu:

- Việt hóa chữ trong giao diện chính.
- Tăng font chữ ở bảng, thẻ thống kê và chú thích biểu đồ để dễ nhìn hơn.
- Chú thích biểu đồ chi tiêu theo danh mục được làm lớn hơn và giãn dòng để vừa khung.
- Ô nhập tiền tự định dạng dấu chấm hàng nghìn, ví dụ nhập `50000000` sẽ hiện `50.000.000`.
- Có bộ lọc giao dịch theo tháng, loại, danh mục và từ khóa.
- Có chức năng xóa hạn mức ngân sách.

## Chạy bằng Qt Creator

Mở file `CMakeLists.txt`, chọn kit Qt Desktop, sau đó Build/Run target `opep_gui`.

## Chạy bằng MSYS2/terminal có Qt + CMake

```bash
cd /c/Users/mon68/Downloads/OPEP_Qt_App
make run-gui
```

Hoặc chạy thủ công:

```bash
cmake -S . -B build-gui
cmake --build build-gui
./build-gui/opep_gui.exe
```

Dữ liệu được lưu trong thư mục `data/`.
