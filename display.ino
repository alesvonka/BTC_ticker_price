void set_display()
{
  display.clearDisplay();

  display.setTextSize(1);
  digitalClockDisplay();
  
  display.setCursor(20, 14);
  display.setTextSize(2);
  display.println(crypto + "/" + curency);

  display.setCursor(0, 38);
  display.setTextSize(3);
  display.print(curencySymbol);
  display.println(price);

  display.display();
}
