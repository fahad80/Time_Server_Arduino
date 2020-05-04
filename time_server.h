/***********************************************************************
 * @file       : time_server.h
 * @author     : Fahad Mirza (fahadmirza8@gmail.com)
 * @version    : V1.0
 * @brief      : Time server driver
 ***********************************************************************/

#ifndef TIME_SERVER_H_
#define TIME_SERVER_H_

#include <stdbool.h>

/*** Public Typedef ****************************************************/
// Timer Event struct
typedef struct sTimerEvent
{
  unsigned long previousMillis; //<! Last Mills value to compare with mills()
  long Interval;                //<! Interval before execute callback()
  bool IsRunning;               //<! Is the timer currently running?
  bool Repeatable;
  void (*Callback)(void);       //<! Timer IRQ callback function
  struct sTimerEvent *Next;     //<! Pointer to the next Timer object.
}sTimerEvent_t;

/*** Public Functions Declarations *************************************/



/*********************************************************************** 
 * @brief    Handles the time for all timer event
 * @details  Call this fucnction in loop()
 * @param    None
 * @return   None
 ***********************************************************************/
void Timer_Handler(void);

/*********************************************************************** 
 * @brief    Print timer instance 
 * @details  Debug function to print all timer instances' address in 
 *           the linked list
 * @param    None
 * @return   None
 ***********************************************************************/
void Timer_PrintAllInstance(void);


// Function pointer typedef. To use inside TimerEvent class
typedef void (*Callback)(void);

/**************************************************************************/
/*! 
    @brief  Class that stores state and functions for interacting with Time Server
*/
/**************************************************************************/
class TimerEvent
{
  private:
    uint32_t Previous_ms; // Last Mills value to compare with mills()
    uint32_t Interval_ms;    // Interval before execute callback()
    boolean  IsRunning;      // Is the timer currently running?
    boolean  Repeat;         // Is this event needs to be repeated
    Callback Cb;             // Timer IRQ callback function

  public:
    /* 
     * Timer event initialization.
     * Param :   interval - Time interval for callback,
     *           cb       - Callback function pointer
     *           repeat   - Should the event repeat after executing callback
     * Return:   none
     */
    TimerEvent(uint32_t interval_ms, Callback cb, boolean repeat = false);
    TimerEvent(void);
    ~TimerEvent();

    void setCallback(Callback cb);
    
    /*
     * Set timer event's interval
     * Param :   interval_ms - interval in milliseconds
     * Return:   none
     */
    void setInterval(uint32_t interval_ms);

    /* 
     * Start a particular timer event.
     * Param :   interval_ms - in milliseconds
     * Return:   none
     */
    void start(uint32_t interval_ms);
    void start(void);

    /* 
     * Stop the timer event and remove from the linked list.
     * Param :   none
     * Return:   none
     */
    void stop(void);

    /*
     * Restart a timer event by stopping the timer event and 
     * then starting it again
     * Param :   none
     * Return:   none
     */
    void restart(void);
}
#endif // TIME_SERVER_H_
