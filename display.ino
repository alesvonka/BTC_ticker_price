void set_display()
{
  display.clearDisplay();

  display.setCursor(20, 10);
  display.setTextSize(2);
  display.println(crypto + "/" + curency);

  display.setCursor(0, 36);
  display.setTextSize(3);
  display.print(curencySymbol);
  display.println(price);

  display.display();
}
