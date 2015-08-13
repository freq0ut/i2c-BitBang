int SCLpin = 3, SDApin = 2, readPin = A0, shft_clr = 8, shft_clk = 9, shft_latch = 10, shft_data = 11, i, j, k, l, m, n, z, counter, address;
//int pause = 250, shortpause = 250, longpause = 1000; //slow mode
int pause = 250, shortpause = 250, longpause = 250;   //fast mode

char incomingByte;
boolean stringComplete;
int register_number, eeprom_number, data_number;
int high_byte[8];
int low_byte[8];
int eeprom_address[8];
int data_byte[8];
byte flag = 0;
byte rw_toggle;
float num2bin, next_num2bin, reg_address, num2bin_used, decode_bin;

String inputString = "";           // a string to hold incoming data
String prompt =">";

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////  Setup  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void setup()
{
  pinMode(SCLpin, OUTPUT); 
  pinMode(SDApin, OUTPUT);
  pinMode(readPin, INPUT);
  digitalWrite(SCLpin, LOW);  
  digitalWrite(SDApin, LOW);
  
  pinMode(shft_clr, OUTPUT);
  pinMode(shft_clk, OUTPUT);
  pinMode(shft_latch, OUTPUT);
  pinMode(shft_data, OUTPUT);
  
  digitalWrite(shft_clr, HIGH);
  digitalWrite(shft_clk, LOW);
  digitalWrite(shft_latch, LOW);
  digitalWrite(shft_data, LOW);
  
  eeprom_address[7] = 1;
  eeprom_address[6] = 0;
  eeprom_address[5] = 1;
  eeprom_address[4] = 0;
  Serial.begin(9600);
  serial_menu();
  Serial.print(prompt);  
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////  Main Program Loop   //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void loop()
{
  USB();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////   Function: USB Serial Data   /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void USB(void)
{
   while (Serial.available() > 0)              // Scan serial port for activity
   { 
     serial_accum();
   }     
   if (stringComplete == true)                 // Checks to see if we have completed text
   {
     serial_commands();                        // Go To serial commands sub
     reset_serial();                           // Reset data once I am done doing something with it
   }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////   Function: Accumlate Serial Data   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void serial_accum(void) // This function accumulated Serial input data for reading back
{
  char inChar = (char)Serial.read();                                           // Flush serial data into variable inChar
  
  if (inChar == 8 && inputString.length() > 0)                                 // Check for backspace
  {
     Serial.write(inChar);                                                     // Echo Data Back to serial port
     Serial.write(" ");                                                        // Echo Blank Data Back To Serial Port
     Serial.write(8);                                                          // Echo backspace Back To Serial Port
     inputString = inputString.substring(0, inputString.length() - 1);         // Subtract character from string object     
  }
  if (inChar == 13)                                                            // Check for line feed
  {
    Serial.write(inChar);                                                      // Echo Data Back to serial port
    Serial.write(char(13));                                                    // Write a line feed and set stringComplete to true
    stringComplete = true;                                                     // stringComplete becomes True
  }
  if (inChar == 10)                                                            // Check for line feed
  {
    stringComplete = true;                                                     // stringComplete becomes True
  }
  if (inChar != 10 && inChar != 8 && inChar != 13)                             // If both carrage return and backspace weren't pressed then do the following
  {
    Serial.write(inChar);                                                      // Echo Data Back to serial port
    inputString += inChar;                                                     // Accumulate Data into inputString
  }                         
} 

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////   Function: Reset the input serial string   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void reset_serial(void) // Clear Serial output with prompt
{
   inputString = "";   // clear the string:
   Serial.print("\n");
   Serial.print(prompt);                                                       // Print prompt
   stringComplete = false;                                                     // Reset 
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////   Function: Menu Navigation   /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void serial_menu(void)//Main Serial Menu
{ 
  Serial.println("");
  Serial.println(" ________________________________________________________  ");
  Serial.println("|********************************************************|"); 
  Serial.println("|***************** EEPROM I2C COMMANDS ******************|"); 
  Serial.println("|********************************************************|"); 
  Serial.println("| Write        = Write to an EEPROM address.             |");  
  Serial.println("| Read         = Read the contents of an EEPROM address. |");
  Serial.println("| ShiftDemo    = Display a light show.                   |");
  Serial.println("| ?            = Command List                            |");
  Serial.println("|________________________________________________________|");
  Serial.println("");
  flag = 1;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////   Function: Handle incoming serial commands   /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void serial_commands(void) //Check serial command inputs
{
  inputString.toLowerCase();                 // Converts the inputstring to lowercase - easier to handle in if then statements
  flag = 0;
  
  char setpoint_string[32];
  inputString.toCharArray(setpoint_string,32);
  
  if(inputString == "?")
  {
    serial_menu();                          // Displays Menu Commands
  } 
  
  if (inputString == "write")
  {
    rw_toggle = 0;
    i2c_write();	
    flag = 1;    
  } 
  
  if(inputString == "read")
  {
    rw_toggle = 1;
       
    i2c_read();    
    flag = 1;    
  } 
  
  if(inputString == "shiftdemo1")
  {
    shift_demo_one();   
    flag = 1;    
  } 
  
  if(inputString == "shiftdemo2")
  {
    shift_demo_two();   
    flag = 1;    
  } 
  
  if(inputString == "shiftdemo3")
  {
    shift_demo_three();   
    flag = 1;    
  } 
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////   Function: i2c Int to Binary for EEPROM ID   /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void num_to_binary_eeprom(void)
{
  byte error = 1;
  while(error == 1)
  { 
    byte exit = 0; 
    reset_serial_no_prompt();                                                   // Reset Serial port input variable without re-displaying prompt
    Serial.println("");
    if(rw_toggle == 0)
      Serial.print("What EEPROM chip do you want to write to? (1-8): ");
    else
      Serial.print("What EEPROM chip do you want to read from? (1-8): ");
    while (exit < 1)                                                            // Makes sure exit condition is 0
    {
      while (Serial.available() > 0)                                            // If Serial Data is avaiable
      { 
        serial_accum();                                                         // Accumulate Serial Data        
        if (stringComplete == true)                                             // Checks to see if we have completed text
          exit = 1;                                                             // Set exit condition to 1 
      }  
    }
    
    num2bin = inputString.toInt();
    if(num2bin <= 8 && num2bin > 0) //8 bit number
    {
      eeprom_number = inputString.toInt();
      num2bin_used = num2bin - 1; //subtract one to get 0-7 instead of 1-8
      for(i=1;i<=3;i++)
      {
        next_num2bin = num2bin_used/2;
        if((int)num2bin_used%2 != 0)
        {
          next_num2bin = next_num2bin - 0.5;
          eeprom_address[i] = 1;
        }
        else
        {
          eeprom_address[i] = 0;  
        }
        num2bin_used = next_num2bin;
      }
      error = 0;
    }
    else
    {
      Serial.println("");
      Serial.println("Invalid entry. Must choose value between 1 and 8.");
      error = 1;
    }
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////   Function: Int to Binary for Register Address   //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void num_to_binary_add(void)
{
  byte error = 1;
  while(error == 1)
  {
    byte exit = 0;
    reset_serial_no_prompt();
    if(rw_toggle == 0)
      Serial.print("What register do you want to write to? (0-32767): ");
    else
      Serial.print("Which register do you want to read? (0-32767): ");
    while (exit < 1)                                                         // Makes sure exit condition is 0
    {
      while (Serial.available() > 0)                                         // If Serial Data is avaiable
      { 
        serial_accum();                                                      // Accumulate Serial Data        
        if (stringComplete == true)                                          // Checks to see if we have completed text
          exit = 1;                                                          // Set exit condition to 1 
      }
    }  
    
    num2bin = inputString.toInt();
    Serial.println("");
    if(num2bin <= 255 && num2bin >= 0) //8 bit number
    {
      register_number = inputString.toInt();
      num2bin_used = num2bin;
      for(i=0;i<=7;i++)
      {
        high_byte[i] = 0;
      }
      //Serial.println("");
      for(i=0;i<=7;i++)
      {
        next_num2bin = num2bin_used/2;
        if((int)num2bin_used%2 != 0)
        {
          next_num2bin = next_num2bin - 0.5;
  
          low_byte[i] = 1;
        }
        else
        {
          low_byte[i] = 0;  
        }
        num2bin_used = next_num2bin;
      }
      error = 0;
    }
    else if(num2bin > 255 && num2bin <= 32767) //16 bit number
    {
      register_number = inputString.toInt();
      num2bin_used = num2bin;
      for(i=0;i<=15;i++)
      {
        next_num2bin = num2bin_used/2;
        if((int)num2bin_used%2 != 0)
        {
          next_num2bin = next_num2bin - 0.5;
          if(i<=7)
            low_byte[i] = 1;
          else
            high_byte[i-8] = 1;
        }
        else
        {
          if(i<=7)
            low_byte[i] = 0;
          else
            high_byte[i-8] = 0;  
        }
        num2bin_used = next_num2bin;
      }
      error = 0;
    }
    else if(num2bin < 0 || num2bin > 32767) //error case
    {
      Serial.println("");
      Serial.println("Invalid entry. Must choose value between 0 and 32767.");
      error = 1;
    }
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////   Function: i2c Int to Binary for Data Byte   /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void num_to_binary_data(void)
{
  byte error = 1;
  while(error == 1)
  {
    byte exit = 0;
    reset_serial_no_prompt();
    Serial.print("What value do you want to store? (0-255): ");
    while (exit < 1)                                                         // Makes sure exit condition is 0
    {
      while (Serial.available() > 0)                                         // If Serial Data is avaiable
      { 
        serial_accum();                                                      // Accumulate Serial Data        
        if (stringComplete == true)                                          // Checks to see if we have completed text
          exit = 1;                                                          // Set exit condition to 1 
      }
    }
    
    num2bin = inputString.toInt();
    
    if(num2bin <= 255 && num2bin >= 0)                                       //8 bit number
    {
      data_number = inputString.toInt();
      num2bin_used = num2bin;
      for(i=0;i<=7;i++)
      {
        next_num2bin = num2bin_used/2;
        if((int)num2bin_used%2 != 0)
        {
          next_num2bin = next_num2bin - 0.5;
          data_byte[i] = 1;
        }
        else
        {
          data_byte[i] = 0;  
        }
        num2bin_used = next_num2bin;
      }
      error = 0;
    }
    else
    {
      Serial.println("");
      Serial.println("Invalid entry. Must choose value between 0 and 255.");
      error = 1;
    }
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////   Function: i2c Start Bit   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void startBit(void)
{
  digitalWrite(SDApin, HIGH); //start bit (SDA goes low)
  delay(shortpause);
  digitalWrite(SCLpin, HIGH); //clock goes low... now ready to receive first byte
  delay(pause);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////   Function: i2c Write 0   /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void writeZero(void)
{
  digitalWrite(SDApin, HIGH);  //data low
  delay(shortpause);  
  digitalWrite(SCLpin, LOW);  //clock high
  delay(shortpause);
  digitalWrite(SCLpin,HIGH); //clock low
  delay(pause);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////   Function: i2c Write 1   /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void writeOne(void)
{
  digitalWrite(SDApin, LOW);  //data high
  delay(pause);
  digitalWrite(SCLpin, LOW);  //clock high
  delay(shortpause);
  digitalWrite(SCLpin,HIGH); //clock low
  delay(pause);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////   Function: i2c Stop Bit   ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void stopBit(void)
{
  digitalWrite(SDApin, HIGH); //SDA goes off
  delay(shortpause);
  digitalWrite(SCLpin, LOW);  //SCL goes on
  delay(pause);
  digitalWrite(SDApin, LOW);  //SDA goes on
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////   Function: i2c Acknowledge   /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ack(void)
{
  digitalWrite(SDApin, LOW);  //data high
  delay(pause);
  digitalWrite(SCLpin, LOW);  //clock high
  delay(shortpause);
  digitalWrite(SCLpin,HIGH); //clock low
  delay(pause);
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////   Function: i2c Cycle Clock   /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void clock(void)
{
  digitalWrite(SCLpin, LOW);  //clock high
  delay(longpause);
  digitalWrite(SCLpin,HIGH); //clock low
  delay(longpause);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////   Function: Reset the Serial Prompt   /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void reset_serial_no_prompt()                                                     //Clear Serial output with no prompt
{
  inputString = "";                                                               // clear the string:
  stringComplete = false;                                                         // Reset 
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////   Function: Create Spaces   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void spaces()// Adding Saces in the serial port
{
  Serial.println('\n');  // Linefeed
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////   Function: i2c Write to EEPROM   /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void i2c_write(void)
{
  num_to_binary_eeprom();
  eeprom_address[0] = 0;
    
  Serial.println("");
  Serial.println("");
    
  num_to_binary_add();
  Serial.println("");
  num_to_binary_data();
  
  startBit();
  i2c_eeprom_ID();
  i2c_eeprom_AD();
  i2c_eeprom_DATA();
  stopBit();
  
  Serial.println("");
  Serial.println("");
  Serial.println("------------------------------------------------");
  Serial.print("The byte with value ");
  Serial.print(data_number);
  Serial.print(":");
  Serial.println("");
  Serial.print("[");
  for(i=7;i>=0;i--)
  {
    Serial.print(data_byte[i]);
    if(i>0)
      Serial.print(" ");  
  }
  Serial.print("]");
  Serial.println("");
  Serial.println("");
  Serial.print("Was stored in EEPROM register # ");
  Serial.print(register_number);
  Serial.print(": ");
  Serial.println("");
  Serial.print("[");
  for(i=7;i>=0;i--)
  {
    Serial.print(high_byte[i]);
    if(i>0)
      Serial.print(" ");  
  }
  Serial.print("]");
  Serial.print("    ");
  Serial.print("[");
  for(i=7;i>=0;i--)
  {
    Serial.print(low_byte[i]);
    if(i>0)
      Serial.print(" ");
  }
  Serial.print("]");
  Serial.println("");
  Serial.println("");
  Serial.print("On EEPROM # ");
  Serial.print(eeprom_number);
  Serial.print(": ");
  Serial.println("");
  Serial.print("[");
  for(i=7;i>=0;i--)          //eeprom
  {
    Serial.print(eeprom_address[i]);
    if(i>0)
      Serial.print(" ");  
  }
  Serial.println("]");
  Serial.println("------------------------------------------------");
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////   Function: i2c Read from EEPROM   ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void i2c_read(void)
{
  num_to_binary_eeprom();
  Serial.println("");
  Serial.println(""); 
  num_to_binary_add();
  startBit();
  eeprom_address[0] = 0;
  i2c_eeprom_ID();
  i2c_eeprom_AD();
  stopBit();
  delay(5);
  startBit();
  eeprom_address[0] = 1;   //turn back on
  delay(5);                //turn back on
  for(i=7;i>=0;i--)                     //turn back on FROM HERE
  {
    if(eeprom_address[i] == 0)
    {
      writeZero();
    }
    else
    {
      writeOne();
    }
  }
  decode_bin = 0;
  k = 7;
  shift_clear();
  latch();
  for(i=0;i<=7;i++)
  {    
    clock();
    if(analogRead(readPin) > 500)
    {
      Serial.print("1 ");
      shiftOne();
      latch();
      decode_bin = decode_bin + pow(2,k);
    }
    else
    {
      shiftZero();
      latch();
      Serial.print("0 ");
      
    }
    k = k - 1;
  }
  
  Serial.println("");
  delay(250);
  Serial.println("");
  Serial.print("The deciaml value stored in register ");
  Serial.print(register_number);
  Serial.print(" on EEPROM #");
  Serial.print(eeprom_number);
  Serial.print(" is: ");
  Serial.println((int)(decode_bin+0.5));
  
  digitalWrite(SCLpin,LOW);              //turn back on TO HERE
  delay(5);
  digitalWrite(SCLpin,HIGH);
  delay(5);
  digitalWrite(SCLpin,LOW);
  delay(5);
  digitalWrite(SDApin,LOW);
  latch();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////   Function: i2c Clock in the EEPROM ID   //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void i2c_eeprom_ID(void)
{
  for(i=7;i>=0;i--)
  {
    if(eeprom_address[i] == 0)
      writeZero();
    else
      writeOne();
  }
  ack();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////   Function: i2c Clock in the Register Number   ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void i2c_eeprom_AD(void)
{
  for(i=7;i>=0;i--)
  {
    if(high_byte[i] == 0)
      writeZero();
    else
      writeOne();
  }
  ack(); 
  for(i=7;i>=0;i--)
  {
    if(low_byte[i] == 0)
      writeZero();
    else
      writeOne();
  }
  ack();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////   Function: i2c Clock in the Data   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void i2c_eeprom_DATA(void)
{
  for(i=7;i>=0;i--)
  {
    if(data_byte[i] == 0)
      writeZero();
    else
      writeOne();
  }
  ack();
}

void shift_clock(void)
{
  digitalWrite(shft_clk, HIGH);
  delay(5);
  digitalWrite(shft_clk, LOW);
  delay(5);
}

void latch(void)
{
  digitalWrite(shft_latch, HIGH);
  delay(5);
  digitalWrite(shft_latch, LOW);
  delay(5);
}

void shiftOne(void)
{
  digitalWrite(shft_data, HIGH);
  delay(5);
  shift_clock();
  digitalWrite(shft_data, LOW);
  delay(5);
}

void shiftZero(void)
{
  digitalWrite(shft_data, LOW);
  delay(5);
  shift_clock();
  delay(5);
}

void shift_clear(void)
{
  digitalWrite(shft_clr, LOW);
  delay(5);
  digitalWrite(shft_clr, HIGH);
  delay(5);
  //latch();
}

void shift_demo_one(void)
{
  shift_clear();
  latch();
  for(i=0;i<=20;i++)
  {
    for(j=0;j<7;j++)
    {
      shiftZero();
      latch();
    }
    shiftOne();
    latch();
  }
  shift_clear();
  latch();  
}

void shift_demo_two(void)
{
  shift_clear();
  latch();
  for(i=0;i<=10;i++)
  {
    for(j=0;j<4;j++)
    {
      shiftOne();
      shiftZero();
    }
    latch();
    delay(150);  
    for(j=0;j<4;j++)
    {
      shiftZero();
      shiftOne();
    }
    latch();
    delay(150);
  }
}

void shift_demo_three(void)
{
  for(k=0;k<=5;k++)
  { 
    for(l=7;l>=0;l--)
    {
      shift_clear();
      shiftOne();
      //latch();
      for(j=0;j<l;j++)
      {
        shiftZero();
        //latch();
      }
      latch();
      delay(50);
    }
//    Serial.println("Check here...");  
//    delay(1000);
    for(i=1;i<=7;i++)
    {
      //shift_clear();
      counter = 0;
      if(i == 1)
        m = 7;
      if(i == 2)
        m = 6;
      if(i == 3)
        m = 5;
      if(i == 4)
        m = 4;
      if(i == 5)
        m = 3;
      if(i == 6)
        m = 2;
      if(i == 7)
        m = 1;
      
      for(j=m;j>=1;j--)
      {
        shift_clear();
        k = i;
        z = m-counter;
        
        //first part
        if(counter>1)
        {
          for(n=0;n<counter-1;n++)
          { 
            shiftZero;
          }     
        }    
        //second part
        if(counter>0)
        {
          shiftOne();
        }
        
        //thrid part  
        while(z>0) //shift in zeros
        {
          shiftZero();
          z = z - 1;
        }
        
        //fourth part
        while(k>0) //shift in MSB ones.
        {
          shiftOne();
          k=k-1;
        }
        
  
        latch();
        counter = counter + 1;
      }
      //delay(50);
    }
    shiftOne();
    delay(150);
    latch();
  }   
}


