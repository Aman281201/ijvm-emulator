#include <stdio.h>  // for getc, printf
#include <stdlib.h> // malloc, free
#include "ijvm.h"
#include "util.h" // read this file for debug prints, endianness helper functions
#include<assert.h>

int32_t max(int32_t i1, int32_t i2)
{
    if(i1 > i2)
      return i1;
  else return i2;
}
typedef struct
{
  uint32_t magic_number;
  uint32_t const_pool_size;
  int32_t *const_pool_data;
  uint32_t text_size;
  byte_t *text_data;
}Info;

int32_t* local_variables;
lv_offset = 0;
lv_cap = 0;
int lv_pointer;
Info* data = NULL;

typedef struct {
    int32_t* stk;
    int size;
    int max_size;
} Stack;

uint32_t program_counter = 0;
bool halted = false;

Stack* stack = NULL;
void stack_init(){
  stack = (Stack*)malloc(sizeof(Stack));
  stack->stk = malloc(10 * sizeof(int32_t));
  stack->max_size = 10;
  stack->size = 0;
}

void push(int32_t value) {
    if (stack->size == stack->max_size) {
        // If stack is full, increase capacity using realloc
        stack->max_size *= 2;
        stack->stk = realloc(stack->stk, stack->max_size * sizeof(int32_t));
    }
    
    stack->stk[stack->size++] = value;
}

int32_t pop() {
    if(stack->size <= 0)
    {printf("Stack underflow!");return -1;}
  else
    return stack->stk[--stack->size];
}

int32_t top() {
    assert(stack->size > 0 && "Stack underflow!");
    return stack->stk[stack->size - 1];
}



void return_util();
void invoke_virtual_util();


FILE *in;   // use fgetc(in) to get a character from in.
            // This will return EOF if no char is available.
FILE *out;  // use for example fprintf(out, "%c", value); to print value to out

void set_input(FILE *fp) 
{ 
  in = fp; 
}

void set_output(FILE *fp) 
{ 
  out = fp; 
}

uint32_t swap_u32(uint32_t num)
{
    return ((num >> 24) & 0xff) | ((num << 8) & 0xff0000) | ((num >> 8) & 0xff00) | ((num << 24) & 0xff000000);
}

int init_ijvm(char *binary_path) 
{
  if(data!=NULL)
    return -1;

  halted=false;
  program_counter=0;
  lv_pointer = 0;
  data = (Info*)malloc(sizeof(Info));
  FILE* fp = fopen(binary_path, "r");
  stack_init();
  uint32_t mnum, cps, ts, temp;
  fread(&mnum, sizeof(uint32_t), 1, fp);
  mnum = swap_u32(mnum);
  data->magic_number = mnum;


  fread(&temp, sizeof(uint32_t), 1, fp);


  fread(&cps, sizeof(uint32_t), 1, fp);
  cps = swap_u32(cps);
  data->const_pool_size = cps;


  data->const_pool_data = (int32_t*)malloc(cps);

  fread((data->const_pool_data),sizeof(int32_t),cps/(sizeof(uint32_t)),fp);


  for(int i = 0 ; i < cps/sizeof(int); i++){
    data->const_pool_data[i] = swap_u32(data->const_pool_data[i]);

  }

  //origin
  fread(&temp, sizeof(uint32_t),1, fp);

  fread(&ts,sizeof(uint32_t),1,fp);
  ts = swap_u32(ts);
  
  data->text_size = ts;
  
  data->text_data = (byte_t*)malloc(ts);

  
  fread((data->text_data),sizeof(byte_t),ts,fp);

  local_variables = (int32_t*)malloc(sizeof(int32_t)*50000);

  lv_cap = 0;
  lv_offset = 0;
  
  if(data != NULL)
  return 0;
  else
  return -1;
}

void destroy_ijvm(void) 
{
  free(data);
  data=NULL;
  stack=NULL;
  program_counter = 0;
  
}

byte_t *get_text(void) 
{
  if(data!=NULL)
    return data->text_data;
  return NULL;
}

unsigned int get_text_size(void) 
{
  if(data!=NULL)
  return data->text_size;

  return 0;
}

word_t get_constant(int i) 
{
  return data->const_pool_data[i];
}

unsigned int get_program_counter(void) 
{
  return program_counter;
}

word_t tos(void) 
{
  return top();
}

bool finished(void) 
{
  return halted;
}

word_t get_local_variable(int i) 
{
  
  return local_variables[i+lv_offset];
}

void goto_util()
  {
    short offset = (short)((data->text_data[program_counter+1] << 8) | (data->text_data[program_counter+2]));

    int pc = 0;
    
   program_counter = program_counter + offset-1;

  }

void step(void) 
{
    if(halted==true)
      return;
  byte_t inst = data->text_data[program_counter];

      switch (inst) {
        case OP_BIPUSH:{
            push((int8_t)(data->text_data[program_counter+1]));
            program_counter++;
            break;
        }
        case OP_DUP:{
            push(top());
            break;
        }
        case OP_ERR:{
            printf("Error\n");halted=true;
            program_counter = -1;
            break;
        }
        case OP_GOTO:{
            goto_util();
            break;
        }
        case OP_HALT:{
            halted=true;
            program_counter = -1;
            break;
          }
        case OP_IADD:{
            int32_t b1,b2;
            b1 = pop();
            b2 = pop();
            push(b1+b2);
            break;
        }
        case OP_IAND:{
            int32_t b1,b2;
            b1 = pop();
            b2 = pop();
            push(b1&b2);
            break;
          }
        case OP_IOR:{
            int32_t b1,b2;
            b1 = pop();
            b2 = pop();
            int8_t b = b1 | b2;
            push(b);
            break;
          }
        case OP_ISUB:{
            int32_t b1,b2;
            b1 = pop();
            b2 = pop();
            push(b2-b1);
            break;
          }
        case OP_POP:{
            pop();
            break;
          }
        case OP_SWAP:{
            int32_t b1,b2;
            b1 = pop();
            b2 = pop();
            push(b1);
            push(b2);
            break;
          }
        case OP_NOP:{
            break;
          }
        case OP_IN:{
            int32_t b;
            char c;
            int32_t x = fread(&c, sizeof(c), 1, in);
            if(x==0)
              push(0);
            else{
            b = (int)(c);
            push(b);
            }
            break;
          }
        case OP_OUT:{
            
            int32_t b;
            b = pop();

          
          char c = (char)(b);
          
          if(out != NULL){
            fwrite(&c,sizeof(c),1, out);
          }
           
            break;
          }
        case OP_IFEQ:{
            int32_t b = pop();
            if(b==0)
              goto_util();
            else{
              program_counter+=2;
            }
            break;
          }
        case OP_IFLT:{
            int32_t b = pop();
            if(b<0)
              goto_util(); 
            else
              program_counter+=2;
            break;
          }
        case OP_IF_ICMPEQ:{
            int32_t b1,b2;
            b1 = pop();
            b2 = pop();
            if(b1==b2)
              goto_util();
            else{
              program_counter+=2;
            }
            break;
          }
        case OP_WIDE:{
          program_counter+=1;
          byte_t inst2 = data->text_data[program_counter];
          switch(inst2)
            {
                case OP_IINC:{
                  uint16_t ind = (data->text_data[program_counter+1] << 8 | data->text_data[program_counter+2]);
                  int8_t val = data->text_data[program_counter+3];
                  
                  local_variables[ind+lv_offset] = local_variables[ind+lv_offset] + val;
                  lv_cap = max(ind+lv_offset,lv_cap);
                  program_counter+=3;
                  break;
                }
              case OP_ILOAD:{
                  uint16_t ind = (data->text_data[program_counter+1] << 8 | data->text_data[program_counter+2]);
                  int32_t load = local_variables[ind+lv_offset];
                  lv_cap = max(ind+lv_offset,lv_cap);
                  push(load);
                  program_counter+=2;
                  break;
                }
              case OP_ISTORE:{
                  uint16_t ind = (data->text_data[program_counter+1] << 8 | data->text_data[program_counter+2]);
                  int32_t store = pop();
                  local_variables[ind+lv_offset] = store;
                  lv_cap = max(ind+lv_offset,lv_cap);
                  program_counter+=2;
                  break;
                }
            }
          break;
        }
        case OP_IINC:{
            byte_t ind = data->text_data[program_counter+1];
            int8_t val = data->text_data[program_counter+2];
            local_variables[ind+lv_offset] = local_variables[ind+lv_offset] + val;
          lv_cap = max(ind+lv_offset,lv_cap);
          program_counter+=2;
            break;
          }
        case OP_ILOAD:{
            byte_t ind = data->text_data[program_counter+1];
            int32_t load = local_variables[ind+lv_offset];
          lv_cap = max(ind+lv_offset,lv_cap);
            push(load);
            program_counter+=1;
            break;
          }
        case OP_INVOKEVIRTUAL:{
            invoke_virtual_util();
            break;
          }
        case OP_IRETURN:{
            return_util();
            break;
          }
        case OP_ISTORE:{
          
            byte_t ind = data->text_data[program_counter+1];
            int32_t store = pop();
            local_variables[ind+lv_offset] = store;
          lv_cap = max(ind+lv_offset,lv_cap);
            program_counter+=1;
            break;
          }
        case OP_LDC_W:{
            uint16_t ind = (data->text_data[program_counter+1] << 8) | (data->text_data[program_counter+2]);
          
            int32_t constant = get_constant(ind);
            push(constant);
            program_counter+=2;
            break;
          }
        default:{
            printf("Unknown opcode\n");
            halted = true;
            program_counter = -1;
            break;
        }
    }

  program_counter++;
}

void run(void) 
{
  while (!finished()) 
  {
    step();
  }
}

byte_t get_instruction(void) 
{ 
  return get_text()[get_program_counter()]; 
}

void invoke_virtual_util()
{
    uint16_t ind = (data->text_data[program_counter+1] << 8 | data->text_data[program_counter+2]);

    int32_t offset = get_constant(ind);
    int32_t prev_p=program_counter+2;
    program_counter=offset;

    uint16_t num_args = (data->text_data[program_counter] << 8 | data->text_data[program_counter+1]);
    uint16_t num_var = (data->text_data[program_counter+2] << 8 | data->text_data[program_counter+3]);
    program_counter +=3;
  
  int32_t* temp_ar = (int32_t*)malloc(sizeof(int32_t)*(num_args-1));

  for(int i = num_args-1; i >=0; i--)
    {temp_ar[i] = pop();}



  push(lv_pointer);
  lv_pointer = stack->size-1;
  push(lv_offset);
  
  lv_offset = lv_cap+1;
  for(int i = 0; i < num_args; i++)
    {local_variables[lv_offset+i] = temp_ar[i];}

  push(num_args + num_var);
  push(prev_p);
    
  
  lv_cap = lv_offset+num_var;
}

void return_util()
{
 
  
   int32_t return_value = pop();
  
  
  int x = stack->size - lv_pointer - 4;

  
  for(int i = 0; i < x ;i++)
    pop();

  
  
  program_counter = pop();
  

  int32_t space = pop();

  if(lv_offset!=0)
  lv_cap = lv_offset -1;

  lv_offset = pop();

  lv_pointer = pop();

  push(return_value);
}
// Below: methods needed by bonus assignments, see ijvm.h

//int get_call_stack_size(void) 
//{
   // TODO: implement me
//   return sp;
//}


// Checks if reference is a freed heap array. Note that this assumes that 
// 
//bool is_heap_freed(word_t reference) 
//{
   // TODO: implement me
// return 0;
//}
