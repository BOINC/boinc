
#define REG register

typedef enum    {Ident_1, Ident_2, Ident_3, Ident_4, Ident_5} Enumeration;

typedef int     One_Thirty;
typedef int     One_Fifty;
typedef char    Capital_Letter;
typedef char    Str_30[31];
typedef int     Arr_1_Dim[50];
typedef int     Arr_2_Dim[50][50];

typedef struct  Record {
        struct Record           *Ptr_Comp;
        Enumeration             Discr;
    union {
          struct {
                  Enumeration Enum_Comp;
                  int         Int_Comp;
                  char        Str_Comp [31];
                  } var_1;
          struct {
                  Enumeration E_Comp_2;
                  char        Str_2_Comp [31];
                  } var_2;
          struct {
                  char        Ch_1_Comp;
                  char        Ch_2_Comp;
                  } var_3;
          } variant;
} Rec_Type, *Rec_Pointer;

struct DS_DATA {
    Rec_Pointer       _Ptr_Glob;
    Rec_Pointer       _Next_Ptr_Glob;
    int             _Int_Glob;
    bool            _Bool_Glob;
    char            _Ch_1_Glob;
    char            _Ch_2_Glob;
    Arr_1_Dim       _Arr_1_Glob;
    Arr_2_Dim       _Arr_2_Glob;
};


#define Int_Glob dd._Int_Glob
#define Bool_Glob dd._Bool_Glob
#define Ch_1_Glob dd._Ch_1_Glob
#define Ch_2_Glob dd._Ch_2_Glob
#define Arr_1_Glob dd._Arr_1_Glob
#define Arr_2_Glob dd._Arr_2_Glob
#define Ptr_Glob dd._Ptr_Glob
#define Next_Ptr_Glob dd._Next_Ptr_Glob

