int gpio_4_value;
int gpio_17_value;
int gpio_27_value;

void* gpio_ctr;

void get_gpio_input_value(void* gpio_ctr, int gpio_nr, int* value);
void init_gpios();