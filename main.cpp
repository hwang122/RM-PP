#include <iostream>
#include <fstream>
#include <cstring>
#include <cstdlib>
#include <iomanip>
#include <ctime>
#include <cmath>

using namespace std;

//task struct
typedef struct task
{
    int ID;//task id
    int period;//period
    int execTime;//execution time
    int priority;//priority
    //copy of the original priority if promotion change the priority
    int c_priority;
    int laxity;//laxity
    int InstanceId;//number of instance
    int relTime;//release time
    int deadline;//absolute deadline
    int execLeft;//time of left execution
}task;

typedef struct data
{
    task *curTask;//which task is assigned
    bool promotion;//promotion or not
    int timeFrom;//start time
    int timeTo;//end time
    int InstanceId;//replication
    int relTime;//replication
    int deadline;//replication
    int execLeft;//replication
}data;

typedef struct schedule
{
    data *taskInfo;//Information of current task
    schedule *next;//next task
}schedule;

/**
 * check the number of tasks
 **/
int countLine(ifstream& in)
{
    string temp;
    int n = 0;
    while(getline(in,temp))
    {
        n++;
    }

    return n;
}

/**
 * read task's parameter from file and
 * assign them to tasks and
 * assign each task's priority
 **/
void readTask(ifstream& in, task t[],int taskNum)
{
    string tempStr;
    char *tempCh, *para;
    int count = 0;

    //read all the tasks
    while(taskNum--)
    {
        //get string from file and convert to char array
        getline(in, tempStr);
        tempCh = new char[tempStr.size()+1];
        tempCh[tempStr.size()]=0;
        memcpy(tempCh, tempStr.c_str(), tempStr.size());

        //get parameter accroding to the delimiter
        para = strtok(tempCh, " (,)");
        t[count].execTime = atoi(para);
        para = strtok(NULL, " (,)");
        t[count].period = atoi(para);
        t[count].priority = 2;//start with 2 to ensure promotion possible
        t[count].ID = count + 1;
        t[count].InstanceId = 1;
        t[count].relTime = 0;
        t[count].deadline = t[count].period;
        t[count].execLeft = t[count].execTime;
        t[count].laxity = t[count].period - t[count].execLeft;

        //assign the priority
        for(int i = 0; i < count; i++)
        {
            if(t[count].period >= t[i].period)
            {
                t[count].priority++;
            }
            else
            {
                t[i].priority++;
            }
        }
        count++;
    }
}

/**
 * calculate the greatest common divisor
 **/
int getGCD(int a, int b)
{
    if(a < b)
    {
        int temp;
        temp = a;
        a = b;
        b = temp;
    }

    int remainder = a%b;
    if(remainder == 0)
    {
        return b;
    }
    else
    {
        a = b;
        b = remainder;
        do
        {
            remainder = a%b;
            a = b;
            b = remainder;
        }
        while(remainder != 0);
        return a;
    }
}

/**
 * calculate the lowest common multiple
 **/
int getLCM(task t[], int taskNum)
{
    int gcd, lcm;

    gcd = getGCD(t[0].period, t[1].period);
    lcm = t[0].period*t[1].period/gcd;

    for(int i = 2; i < taskNum; i++)
    {
        gcd = getGCD(t[i].period, lcm);
        lcm = lcm*t[i].period/gcd;
    }
    return lcm;
}

/**
 * output the result with good format to a file
 **/
void output(schedule *head)
{
    ofstream file;
    file.open("result.txt", ios::trunc | ios::out);

    file <<"Promotion"<< setw(15)<<"Time (from)"<< setw(13)<<"Time (to)"<< setw(11)<<"Task Id"
         << setw(15)<< "Instance Id"<< setw(13)<<"Rel. Time"<< setw(12)<<"Deadline"
         << setw(19)<<"Execution Left" << endl;

    //output the result
    schedule *node = head;
    while(node != NULL)
    {
        if(node->taskInfo->curTask == NULL)
        {
            file << "No task"<<endl;
        }
        else
        {
            if(node->taskInfo->promotion)
                file << "Yes"<<setw(12);
            else
                file << "No"<<setw(13);

            file << node->taskInfo->timeFrom;
            file << setw(15)<<node->taskInfo->timeTo;
            file << setw(13)<<node->taskInfo->curTask->ID;
            file << setw(11)<<node->taskInfo->InstanceId;
            file << setw(15)<<node->taskInfo->relTime;
            file << setw(13)<<node->taskInfo->deadline;
            file << setw(13)<<node->taskInfo->execLeft<<endl;
        }
        node = node->next;
    }
}

/**
 * pseudo code for RM-PP schedule
 * if some job is released then
 *  assign RM priority to this job
 * endif
 * if the laxity of some job beome 0
 *  assign this job with highest priority
 * endif
 **/
void RM_PP(task t[], schedule *head, int taskNum)
{
    //count the lowest common multiple
    int LCM = getLCM(t, taskNum);
    //used to store the priority
    int cur_priority;
    bool promotion = false, new_task = false, check = false,flag = false;
    int num_promotion;
    //define the head of schedule list
    schedule *cur = head;

    //assign the information for head
    for(int i = 0; i < taskNum; i++)
    {
        //find the task with highest priority
        if(t[i].priority == 2)
        {
            //store info in current schedule node
            cur->taskInfo->curTask = &t[i];
            cur->taskInfo->promotion = false;
            cur->taskInfo->timeFrom = 0;
            cur->taskInfo->timeTo = t[i].execTime;
            cur->taskInfo->InstanceId = t[i].InstanceId;
            cur->taskInfo->relTime = 0;
            cur->taskInfo->deadline = t[i].period;
            cur->taskInfo->execLeft = 0;
            //change task's related info
            t[i].InstanceId++;
            t[i].relTime += t[i].period;
            t[i].deadline += t[i].period;
            t[i].execLeft = t[i].execTime;
            t[i].laxity = t[i].period - t[i].execLeft;
        }
    }

    for(int i = 0; i < taskNum; i++)
    {
        if(t[i].priority != 2)
        {
            t[i].laxity -= cur->taskInfo->timeTo;
        }
    }

    //continue to schedule until reach the hyper-period
    loop:
    while(cur->taskInfo->timeTo != LCM)
    {
        //get last task's priority
        //last_priority = cur->taskInfo->curTask->priority;
        //point to the next schedule
        schedule *nextTask;
        nextTask = new schedule;
        nextTask->next = NULL;
        cur->next = nextTask;
        nextTask->taskInfo = new data;
        nextTask->taskInfo->timeFrom = cur->taskInfo->timeTo;
        cur = nextTask;
        cur->taskInfo->timeTo = cur->taskInfo->timeFrom;

        /**doing scheduling**/
        //no promotion
        if(!promotion)
        {
            //decide next task's priority
            cur_priority = taskNum + 2;
            //decide which tasks with highest priority is released
            for(int i = 0; i < taskNum; i++)
            {
                if(cur->taskInfo->timeFrom >= t[i].relTime)
                {
                    if(t[i].priority < cur_priority)
                    {
                        cur_priority = t[i].priority;
                    }
                }
            }

            num_promotion = 0;
            for(int i = 0; i < taskNum; i++)
            {
                //if some job has laxity 0, then promote this job
                if(t[i].laxity == 0)
                {
                    promotion = true;
                    flag = true;
                    num_promotion++;
                    //assign the promotion priority to this task
                    t[i].c_priority = t[i].priority;
                    t[i].priority = 1;
                    cur_priority = t[i].priority;
                }
                //if multiple jobs need promote, then this job could not be RM-PP scheduled
                if(num_promotion > 1)
                {
                    cout <<"At time "<<cur->taskInfo->timeTo<<", RM-PP not schedulable!"<<endl;
                    cur->taskInfo->curTask = NULL;
                    goto finish;
                }
            }
        }
        //promotion RM-PP
        else
        {
            flag = true;
            //decide which task need promotion
            for(int i = 0; i < taskNum; i++)
            {
                if(t[i].laxity == 0)
                {
                    //assign the promotion priority to this task
                    t[i].c_priority = t[i].priority;
                    t[i].priority = 1;
                    cur_priority = t[i].priority;
                }
            }
        }

        /**doing tasks**/
        //if no task available
        if(cur_priority == taskNum + 2)
        {
            cur->taskInfo->curTask = NULL;
            cur->taskInfo->timeTo++;
            goto loop;
        }
        //else execute the task
        for(int i = 0; i < taskNum; i++)
        {
            if(t[i].priority == cur_priority)
            {
                cur->taskInfo->curTask = &t[i];
                cur->taskInfo->promotion = promotion;
                cur->taskInfo->InstanceId = t[i].InstanceId;
                cur->taskInfo->relTime = t[i].relTime;
                cur->taskInfo->deadline = t[i].deadline;
                cur->taskInfo->execLeft = t[i].execLeft;
                break;
            }
        }
        //check every time point while executing the task
        int execLeft = cur->taskInfo->curTask->execLeft;
        for(int i = 0; i < execLeft; i++)
        {
            cur->taskInfo->timeTo++;
            cur->taskInfo->execLeft--;
            cur->taskInfo->curTask->execLeft
                = cur->taskInfo->execLeft;

            //change tasks' laxity
            for(int j = 0; j < taskNum; j++)
            {
                if(t[j].priority == cur->taskInfo->curTask->priority)
                {
                    t[j].laxity = cur->taskInfo->deadline - cur->taskInfo->timeTo - cur->taskInfo->execLeft;
                    continue;
                }
                if(cur->taskInfo->timeTo > t[j].relTime)
                    t[j].laxity--;
            }
            //if this task is finihsed
            if(cur->taskInfo->execLeft == 0)
            {
                cur->taskInfo->curTask->InstanceId++;
                cur->taskInfo->curTask->relTime
                    += cur->taskInfo->curTask->period;
                cur->taskInfo->curTask->deadline
                    += cur->taskInfo->curTask->period;
                cur->taskInfo->curTask->execLeft
                    = cur->taskInfo->curTask->execTime;
                cur->taskInfo->curTask->laxity
                    = cur->taskInfo->curTask->period
                    - cur->taskInfo->curTask->execLeft;
                //if this task is promoted, reset its priority
                if(promotion)
                {
                    cur->taskInfo->curTask->priority
                        =cur->taskInfo->curTask->c_priority;
                }
                promotion = false;
                goto loop;
            }

            //if there is no promotion, one need to check all the time points
            if(!promotion)
            {
                num_promotion = 0;
                new_task = false;
                check = false;
                //check other tasks at this time point
                for(int j = 0; j < taskNum; j++)
                {
                    //start another task whenever either conditon below happens
                    //task with 0 laxity, promotion needed
                    if(t[j].laxity == 0
                       &&t[j].priority > cur->taskInfo->curTask->priority)
                    {
                        num_promotion++;
                        new_task = true;
                        promotion = true;
                    }
                    //task with higher priority released
                    if(t[j].relTime <= cur->taskInfo->timeTo
                       &&t[j].priority < cur->taskInfo->curTask->priority)
                    {
                        new_task = true;
                        //current task has laxity 0
                        if(cur->taskInfo->curTask->laxity == 0)
                        {
                            //this if clause can only run once
                            if(!check)
                            {
                                check = true;
                                num_promotion++;
                            }
                            promotion = true;
                        }
                        else
                            promotion = false;
                    }
                }
                if(num_promotion > 1)
                {
                    cout <<"At time "<<cur->taskInfo->timeTo<<", RM-PP not schedulable!"<<endl;
                    goto finish;
                }
                if(new_task)
                    goto loop;
            }
            //check whether other tasks have promotion
            //if so, this task cannot be RM-PP schedulable
            else
            {
                for(int j = 0; j < taskNum; j++)
                {
                    if(t[j].priority == cur->taskInfo->curTask->priority)
                        continue;

                    if(t[j].laxity == 0)
                    {
                        num_promotion = 2;
                        cout <<"At time "<<cur->taskInfo->timeTo<<", RM-PP not schedulable!"<<endl;
                        goto finish;
                    }
                }
            }
        }
    }

    finish:
    if(flag)
    {
        if(num_promotion > 1)
            cout << "This task is neither RM nor RM-PP schedulable."<<endl;
        else
            cout << "This task is not RM schedulable, RM-PP is used."<<endl;
    }
    else
        cout << "This task is RM schedulable."<<endl;
}

/**
 * This function is going to generate task set randomly
 **/
void taskSetGen(task t[], int taskNum, double util)
{
    srand((unsigned)time(0));
    //make sure the utilization is between 70% and 100%
    int utilization = util * 100;
    double c_utilization = 2.0;
    int exec[taskNum], total = 0;
    double percent, temp;

    for(int i = 0; i < taskNum; i++)
    {
        exec[i] = rand()%(utilization-50) + 50;
        total += exec[i];
    }
    while(abs(c_utilization - (double)utilization/100) > 0.01||c_utilization > 1)
    {
        c_utilization = 0;
        for(int i = 0; i < taskNum; i++)
        {
            t[i].ID = i+1;
            t[i].period = rand()%9 + 4;
            percent = (double)exec[i]*utilization/total/100;
            temp = percent*t[i].period;
            t[i].execTime = (int)(temp-floor(temp)<=0.5? floor(temp):ceil(temp));
            t[i].priority = 2;

            for(int j = 0; j < i; j++)
            {
                if(t[i].period >= t[j].period)
                    t[i].priority++;
                else
                    t[j].priority++;
            }
            t[i].InstanceId = 1;
            t[i].relTime = 0;
            t[i].deadline = t[i].period;
            t[i].execLeft = t[i].execTime;
            t[i].laxity = t[i].period - t[i].execLeft;

            c_utilization += (double)t[i].execTime/t[i].period;
        }
    }

    for(int i = 0; i < taskNum; i++)
    {
        cout<<"Task "<<t[i].ID<<" execution time "<<t[i].execTime<<" period "<<t[i].period<<endl;
    }
    cout<<"Utilization is "<<setprecision(2)<<c_utilization<<endl;
}

int main()
{
    /**initiation**/
    //if need to read a file
    int choice = 0;
    task *t;
    int numTask = 0;
    double utilization = 0;

    cout << "1. Randomly generate task set"<<endl;
    cout << "2. Read task set from a file"<<endl;

    cin >> choice;

    switch(choice)
    {
        case 1:
        {
            cout << "Enter the number of tasks:"<<endl;
            cin >> numTask;
            cout << "Enter the utilization [0.7, 1]:"<<endl;
            cin >> utilization;
            while(utilization > 1||utilization < 0.7)
            {
                cout << "Invalid utilization, enter again:"<<endl;
                cin >> utilization;
            }

            t = new task[numTask];
            taskSetGen(t, numTask, utilization);

            break;
        }
        case 2:
        {
            char taskInitFile[100];

            ifstream file;
            cout << "Input the task file:" << endl;
            cin >> taskInitFile;

            file.open(taskInitFile, ios::in);

            if(file.fail())
            {
                cout << "File is not exist!";
                file.close();
                return -1;
            }
            else
                numTask = countLine(file);

            file.clear();
            file.seekg(ios::beg);

            t = new task[numTask];
            readTask(file, t, numTask);

            for(int i = 0; i < numTask; i++)
                utilization += (double)t[i].execTime/t[i].period;

            if(utilization > 1)
            {
                cout << "Utilization is larger than 1."<<endl;
                return -1;
            }
            break;
        }
        default:
            cout << "Error"<<endl;
            return -1;
    }

    /**end of initiation**/

    /**RM-PP schedule**/
    schedule *head;
    head = new schedule;
    head->taskInfo = new data;
    head->next = NULL;
    RM_PP(t, head, numTask);

    /**Output the result**/
    output(head);

    /**free memory**/
    schedule *node = head;
    while(node != NULL)
    {
        head = node->next;
        delete node->taskInfo;
        delete node;
        node = head;
    }
    delete[] t;

    return 0;
}
