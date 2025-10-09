#include "matrix.h"
#include "quantum.h"

#if (MATRIX_COLS <= 8)
#    define print_matrix_header()  print("\nr/c 01234567\n")
#    define print_matrix_row(row)  print_bin_reverse8(matrix_get_row(row))
#    define ROW_SHIFTER ((uint8_t)1)
#elif (MATRIX_COLS <= 16)
#    define print_matrix_header()  print("\nr/c 0123456789ABCDEF\n")
#    define print_matrix_row(row)  print_bin_reverse16(matrix_get_row(row))
#    define ROW_SHIFTER ((uint16_t)1)
#elif (MATRIX_COLS <= 32)
#    define print_matrix_header()  print("\nr/c 0123456789ABCDEF0123456789ABCDEF\n")
#    define print_matrix_row(row)  print_bin_reverse32(matrix_get_row(row))
#    define ROW_SHIFTER  ((uint32_t)1)
#endif

static const pin_t row_pins[MATRIX_ROWS] = MATRIX_ROW_PINS;
static const pin_t col_pins[MATRIX_COLS] = MATRIX_COL_PINS;

#define PHYS_COLS 8               // 物理列は0..7
#define VIRT_COL_OFFSET 8         // 論理8..15は逆走査で埋める

static inline void select_row(uint8_t r) {
    pin_t p = row_pins[r];
    if (p == NO_PIN) return;
    setPinOutput(p);
    writePinLow(p);               // アクティブLow
}
static inline void unselect_row(uint8_t r) {
    pin_t p = row_pins[r];
    if (p == NO_PIN) return;
    setPinInputHigh(p);           // Hi-Z + プルアップ
}
static inline void unselect_rows(void) {
    for (uint8_t r = 0; r < MATRIX_ROWS; r++) unselect_row(r);
}

static inline void set_cols_input_pullup(void) {
    for (uint8_t c = 0; c < PHYS_COLS; c++) {
        pin_t p = col_pins[c];
        if (p == NO_PIN) continue;
        setPinInputHigh(p);       // 入力＋プルアップ
    }
}
static inline void unselect_col(uint8_t c) {
    pin_t p = col_pins[c];
    if (p == NO_PIN) return;
    setPinInputHigh(p);           // Hi-Z（競合回避）
}
static inline void select_col(uint8_t c) {
    pin_t p = col_pins[c];
    if (p == NO_PIN) return;
    setPinOutput(p);
    writePinLow(p);               // アクティブLow
}

static matrix_row_t scan_cols_on_row(uint8_t r) {
    matrix_row_t bits = 0;
    select_row(r);
    wait_us(30);
    for (uint8_t c = 0; c < PHYS_COLS; c++) {
        pin_t p = col_pins[c];
        if (p == NO_PIN) continue;
        if (!readPin(p)) {
            bits |= (ROW_SHIFTER << c); // 0=押下
        }
    }
    unselect_row(r);
    return bits;
}

static uint16_t read_rows_mask(void) {
    uint16_t mask = 0;
    for (uint8_t r = 0; r < MATRIX_ROWS; r++) {
        pin_t p = row_pins[r];
        if (p == NO_PIN) continue;
        // 行は入力＋プルアップ
        if (!readPin(p)) {
            mask |= (1u << r);
        }
    }
    return mask;
}

void matrix_init_custom(void) {
    unselect_rows();
    set_cols_input_pullup();
}

bool matrix_scan_custom(matrix_row_t current_matrix[]) {
    bool changed = false;

    // 第1パス: ROW -> COL （論理 0..7）
    matrix_row_t new_rows[MATRIX_ROWS] = {0};
    for (uint8_t r = 0; r < MATRIX_ROWS; r++) {
        new_rows[r] = scan_cols_on_row(r);
    }

    // 列をHi-Z、行を入力へ切替
    unselect_rows();
    set_cols_input_pullup(); // 念のため列は入力に戻す
    for (uint8_t r = 0; r < MATRIX_ROWS; r++) {
        pin_t p = row_pins[r];
        if (p == NO_PIN) continue;
        setPinInputHigh(p);
    }

    // 第2パス: COL -> ROW （論理 8..15）
    for (uint8_t c = 0; c < PHYS_COLS; c++) {
        pin_t cp = col_pins[c];
        if (cp == NO_PIN) continue;

        select_col(c);
        wait_us(30);
        uint16_t row_mask = read_rows_mask();
        unselect_col(c);

        for (uint8_t r = 0; r < MATRIX_ROWS; r++) {
            if (row_mask & (1u << r)) {
                new_rows[r] |= (ROW_SHIFTER << (VIRT_COL_OFFSET + c));
            }
        }
    }

    // 状態反映
    for (uint8_t r = 0; r < MATRIX_ROWS; r++) {
        if (current_matrix[r] != new_rows[r]) {
            current_matrix[r] = new_rows[r];
            changed = true;
        }
    }

    // 次回のため安全側に戻す
    unselect_rows();
    set_cols_input_pullup();
    return changed;
}