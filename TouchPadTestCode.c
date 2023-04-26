
void keyPad() {
  //resets screen to white
  tft.fillScreen(ILI9341_WHITE);
  tft.setTextColor(ILI9341_BLACK);
  //Creates the horizontal lines of the keypad for the alarm
  tft.drawLine(0, 118, 240, 118, ILI9341_BLACK);
  tft.drawLine(0, 170, 240, 170, ILI9341_BLACK);
  tft.drawLine(0, 219, 240, 219, ILI9341_BLACK);
  tft.drawLine(0, 271, 240, 271, ILI9341_BLACK);

  //Creates the vertical lines for the keypad for the alarm
  tft.drawLine(80, 118, 80, 320, ILI9341_BLACK);
  tft.drawLine(160, 118, 160, 320, ILI9341_BLACK);

  //Creates entry box for input
  tft.drawRect(24, 11, 198, 65, ILI9341_BLACK);

  //Sets the numbers for the keypad
  tft.setTextSize(3);
  tft.setCursor(35, 135);
  tft.print("1");

  tft.setCursor(115, 135);
  tft.print("2");

  tft.setCursor(195, 135);
  tft.print("3");

  tft.setCursor(35, 185);
  tft.print("4");

  tft.setCursor(115, 185);
  tft.print("5");

  tft.setCursor(195, 185);
  tft.print("6");

  tft.setCursor(35, 235);
  tft.print("7");

  tft.setCursor(115, 235);
  tft.print("8");

  tft.setCursor(195, 235);
  tft.print("9");

  tft.setCursor(115, 285);
  tft.print("0");
  //Sets the text inside the box of the alarm input box

}


void Set_Alarm_Hour(){

  keyPad();

do{
  TSPoint p = ts.getPoint();
  switch(p.y){
    //Case for first row of numbers
    case (p.y>380 && p.y<495):
      switch(p.x){
        case (p.x>180 && p.x<330):
          AlarmHour1 = 1;
        break;

        case (p.x>340 && p.x<615):
          AlarmHour1 = 2;
        break;

        case (p.x>645 && p.x<863):
          AlarmHour1 = 3;
        break;
      }
    break;
    //Case for Second Row of Numbers
    case (p.y>510 && p.y<635):
      switch(p.x){
        case (p.x>180 && p.x<330):
          AlarmHour1 = 4;
        break;

        case (p.x>340 && p.x<615):
          AlarmHour1 = 5;
        break;

        case (p.x>645 && p.x<863):
          AlarmHour1 = 6;
        break;
      }
    break;

    case (p.y>655 && p.y<780):
      switch(p.x){
        case (p.x>180 && p.x<330):
          AlarmHour1 = 7;
        break;

        case (p.x>340 && p.x<615):
          AlarmHour1 = 8;
        break;

        case (p.x>645 && p.x<863):
          AlarmHour1 = 9;
        break;
      }
    break;

    case (p.y>800 && p.y<915):

      switch(p.x){

        case (p.x>340 && p.x<615):
          AlarmHour1 = 0;
        break;
      }
    break;


  }
}while(p.x==NULL);

do{
  TSPoint p = ts.getPoint();
  switch(p.y){
    //Case for first row of numbers
    case (p.y>380 && p.y<495):
      switch(p.x){
        case (p.x>180 && p.x<330):
          AlarmHour2 = 1;
        break;

        case (p.x>340 && p.x<615):
          AlarmHour2 = 2;
        break;

        case (p.x>645 && p.x<863):
          AlarmHour2 = 3;
        break;
      }
    break;
    //Case for Second Row of Numbers
    case (p.y>510 && p.y<635):
      switch(p.x){
        case (p.x>180 && p.x<330):
          AlarmHour2 = 4;
        break;

        case (p.x>340 && p.x<615):
          AlarmHour2 = 5;
        break;

        case (p.x>645 && p.x<863):
          AlarmHour2 = 6;
        break;
      }
    break;

    case (p.y>655 && p.y<780):
      switch(p.x){
        case (p.x>180 && p.x<330):
          AlarmHour2 = 7;
        break;

        case (p.x>340 && p.x<615):
          AlarmHour2 = 8;
        break;

        case (p.x>645 && p.x<863):
          AlarmHour2 = 9;
        break;
      }
    break;

    case (p.y>800 && p.y<915):

      switch(p.x){

        case (p.x>340 && p.x<615):
          AlarmHour2 = 0;
        break;
      }

    break;

      }

  }while(p.x == NULL);

  Set_Alarm_Minute();

}

void Set_Alarm_Minute(){
  keyPad();
}
