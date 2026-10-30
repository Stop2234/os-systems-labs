.text

# Подпрограмма вычисления квадратного корня методом Ньютона
# Формула: x_(n+1) = 0.5 * (x_n + a/x_n)
# Вход: fa0 - число, fa1 - точность
# Выход: fa0 - квадратный корень
calculate_sqrt:
    # Начальное приближение: x0 = a / 2
    fmv.d ft0, fa0        # ft0 = a
    li t0, 2
    fcvt.d.w ft1, t0      # ft1 = 2.0
    fdiv.d ft2, ft0, ft1  # ft2 = x_n = a/2
    
sqrt_loop:
    # Сохраняем предыдущее значение
    fmv.d ft3, ft2        # ft3 = x_prev
    
    # Вычисляем x_(n+1) = 0.5 * (x_n + a/x_n)
    fdiv.d ft4, ft0, ft2  # ft4 = a/x_n
    fadd.d ft4, ft2, ft4  # ft4 = x_n + a/x_n
    fdiv.d ft2, ft4, ft1  # ft2 = 0.5 * (x_n + a/x_n)
    
    # Проверяем точность: |x_n - x_prev| < precision
    fsub.d ft5, ft2, ft3  # ft5 = x_n - x_prev
    fabs.d ft5, ft5       # ft5 = |x_n - x_prev|
    flt.d t1, ft5, fa1    # t1 = (|diff| < precision)
    
    beqz t1, sqrt_loop    # если нет, продолжаем
    
    # Возвращаем результат
    fmv.d fa0, ft2
    ret