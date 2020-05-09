/***********************************************************************
 * @file      time_server.cpp
 * @author    Fahad Mirza (fmirz007@7-11.com) 
 * @version   V1.0
 * @brief     Time server infrastructure
 ***********************************************************************/

/*** Includes **********************************************************/

#include "time_server.h"


typedef struct sTimerEventNode
{
  TimerEvent *timerEvent;
  struct sTimerEventNode *next;
}sTimerEventNode_t;


/*** Private Variables *************************************************/
static sTimerEventNode_t *TimerListHead = NULL;
static uint8_t NumberOfEventsRunning = 0;
static bool TimerInitialized = false;


/*** Private Functions Declarations ************************************/
static void initTimerISR(void);
static void disableTimerISR(void);
static void insertTimerEvent(TimerEvent *obj);
static bool timerEventExists(TimerEvent *obj);
static void removeTimerEvent(TimerEvent *obj);

/*** Functions Definitions *********************************************/
/*********************************************************************** 
 * @brief      Timer event initialization
 * @details    Initializes all the TimerEven_t variables with default
 *             value
 * @param[in]  obj - Timer event object pointer
 * @param[in]  callback - Timer event's callback function pointer
 * @return     None
 ***********************************************************************/
TimerEvent::TimerEvent(Callback cb, uint32_t interval_ms, boolean repeat)
{
  ElapsedTime_ms = 0; 
  Interval_ms = interval_ms;
  IsRunning = false;
  Repeat = repeat; 
  Cb = cb;
}


void TimerEvent::setInterval(uint32_t interval_ms)
{
  Interval_ms = interval_ms;
}


void TimerEvent::start(uint32_t interval_ms)
{
  Interval_ms = interval_ms;
  start();
}


void TimerEvent::start(void)
{
    // If no time is provided, dont include it
    if(Interval_ms == 0)
    {
      return;
    }
    
    // Check if the event is already in the LinkedList
    if(timerEventExists(this) == true)
    {
        if(IsRunning == true)
        {
          // It is already running, don't include
          return;
        }
        
        ElapsedTime_ms = Interval_ms;
        IsRunning = true;
        NumberOfEventsRunning++;
    }
    else
    {
      ElapsedTime_ms = Interval_ms;
      insertTimerEvent(this);
      IsRunning = true;
      NumberOfEventsRunning++;
    }

    if(NumberOfEventsRunning > 0)
    {
      initTimerISR();
    }
}


void TimerEvent::stop(void)
{
  if(IsRunning == true)
  {
    if(NumberOfEventsRunning > 0)
    {
      NumberOfEventsRunning--;
    }
  }
  
  IsRunning = false;
  ElapsedTime_ms = 0;

  if(NumberOfEventsRunning == 0)
  {
    disableTimerISR();
  }
}

/*********************************************************************** 
 * @brief      Restart a timer event
 * @details    Restart a timer event by stopping the timer event and 
 *             then starting it again
 * @param[in]  none
 * @return     none
 ***********************************************************************/
void TimerEvent::restart(void)
{
    stop();
    start();
}

/************************ static functions common to all instances ************************/

/*********************************************************************** 
 * @brief      Check if a TimerEvent instance is already exist  
 *             in the linked list
 * @param[in]  obj - sTimerEvent_t object pointer
 * @return     None
 ***********************************************************************/
static bool timerEventExists(TimerEvent *obj)
{
  sTimerEventNode_t *cur = TimerListHead;

  while(cur != NULL)
  {
    if(cur->timerEvent == obj)
    {
      return true;
    }
    cur = cur->next;
  }

  return false;
}

/*********************************************************************** 
 * @brief      Add timer event to the linked list
 * @param[in]  obj - sTimerEvent_t object pointer
 * @return     None
 ***********************************************************************/
static void insertTimerEvent(TimerEvent *obj)
{
  if(TimerListHead == NULL)
  {
    TimerListHead = new sTimerEventNode_t{obj, NULL};
  }
  else
  {
    // Find the next available space to store
    sTimerEventNode_t *cur = TimerListHead;
    
    while (cur->next != NULL )
    {
       cur = cur->next;
    }

    cur->next = new sTimerEventNode_t{obj, NULL};
  }
}


static void initTimerISR(void)
{
  /*
   * Waveform Generation Mode: Mode 4->CTC
   *      In this mode once timer reaches OCR1A value
   *      it will go back to zero, and start counting again.
   *      
   * Prescaler: 1
   *      The clock will receive the system clocl i.e. 16MHz
   *      
   * Output Compare Register: 16000
   *      To generate 1ms interrupt
   *      
   * Interrupt Mask: Enable OCIE1A interrupt
   *      Generate interrupt when timer reaches OCR1A
   */

  if(TimerInitialized == false)
  {   
  
    // Setting WGM's last two bits to zero
    TCCR1A = 0;
  
    // Setting WGM's other two bits
    TCCR1B &= ~(1 << WGM13);
    TCCR1B |= (1 << WGM12);
  
    // Selecting prescaler 1
    TCCR1B |= (1 << CS10);
    TCCR1B &= ~(1 << CS11);
    TCCR1B &= ~(1 << CS12); 

    // Clear counter register
    TCNT1 = 0;
  
    // Set OCR1A to generate 1ms 
    OCR1A = 16000;

    // Enable the interrupt
    TIMSK1 = (1 << OCIE1A);

    // Enable global interrupt
    sei();

    // Set flag to avoid re-initialization
    TimerInitialized = true;
  }
}

static void disableTimerISR(void)
{
  // Disable the interrupt
  TIMSK1 &= ~(1 << OCIE1A);
  TimerInitialized = false;
}

/*********************************************************************** 
 * @brief    Handles the time for all timer event
 * @details  Call this fucnction in loop()
 * @param    None
 * @return   None
 ***********************************************************************/
ISR(TIMER1_COMPA_vect)
{
  sTimerEventNode_t *cur = TimerListHead;

  // Check which Node's time is expired
  while(cur != NULL)
  {
    if(cur->timerEvent->IsRunning)
    {
      // Decrement 1ms
      cur->timerEvent->ElapsedTime_ms--;
    }
    cur = cur->next;
  }

  // Now take out the expired Nodes and execute their callbacks
  cur = TimerListHead;
  while(cur != NULL)
  {
    if(cur->timerEvent->ElapsedTime_ms == 0)
    {
      // Execute the callback
      if(cur->timerEvent->Cb != NULL)
      {
        cur->timerEvent->Cb();
      }

      if(cur->timerEvent->Repeat)
      {
        cur->timerEvent->ElapsedTime_ms = cur->timerEvent->Interval_ms;
      }
      else
      {
        // Remove the instance from the linkedList
        cur->timerEvent->stop();
      }
    }
    cur = cur->next;
  }        
}

/*********************************************************************** 
 * @brief    Print timer instance 
 * @details  Debug function to print all timer instances' address in 
 *           the linked list
 * @param    None
 * @return   None
 ***********************************************************************/
void Timer_PrintAllInstance(void)
{
  #if DEBUG
  sTimerEvent_t* cur = TimerListHead;
  DBG_Println("PrintAllInstance:");
  
  while(cur != NULL)
  {
    uint32_t add = (uint32_t)cur;
    Serial.print(add, HEX);
    Serial.println();
    cur = cur->Next;
  }

  DBG_Println(F("PrintAllInstance: Done"));
  #endif
}









/*****END OF FILE****/
