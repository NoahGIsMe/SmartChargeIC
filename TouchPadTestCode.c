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
