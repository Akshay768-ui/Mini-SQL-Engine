#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct
{
    int id;
    char name[50];
    int age;
    int marks;
    int deptID;
} Student;

typedef struct
{
    int deptID;
    char department[50];
} Department;

typedef struct
{
    Student rows[100];
    int count;
} ResultSet;

typedef struct
{
    int key;
    Student rows[100];
    int rowCount;
} Group;

typedef struct
{
    Student student;
    Department department;
} JoinedRow;

typedef struct
{
    JoinedRow rows[100];
    int count;
} JoinedResultSet;

typedef struct
{
    char column[20];
    char op[10];
    char value[20];
} WhereCondition;

typedef struct
{
    char command[20];
    char table[50];
    int hasFrom;
    char columns[20][50];
    int columnCount;
    int hasWhere;
    char whereColumn[20];
    char whereOperator[10];
    char whereValue[20];
    WhereCondition whereConditions[10];
    char whereLogicalOps[10][10];
    int whereConditionCount;
    int hasOrderBy;
    char orderColumn[20];
    char orderType[10];
    int hasAggregate;
    char aggregate[20];
    char aggregateColumn[20];
    int hasJoin;
    char joinTable[50];
    int hasGroupBy;
    char groupColumn[20];
    int hasDistinct;
    char leftJoinColumn[50];
    char rightJoinColumn[50];
} Query;

void tokenizeQuery(char *query, char tokens[][50], int *tokenCount)
{
    char *token = strtok(query, " ");

    while (token != NULL)
    {
        strcpy(tokens[*tokenCount], token);
        (*tokenCount)++;
        token = strtok(NULL, " ");
    }
}

void cleanToken(char *token)
{
    int len = strlen(token);
    for (int i = 0; i < len; i++)
    {
        if (token[i] == ';')
        {
            token[i] = '\0';
            break;
        }
    }
}

void normalizeToken(char *token)
{
    for (int i = 0; token[i] != '\0'; i++)
    {
        token[i] = toupper((unsigned char)token[i]);
    }
}

int isAllowedColumn(char *column)
{
    if (strcmp(column, "ID") == 0 || strcmp(column, "NAME") == 0 || strcmp(column, "AGE") == 0 || strcmp(column, "MARKS") == 0 || strcmp(column, "DEPTID") == 0 || strcmp(column, "DEPARTMENT") == 0)
        return 1;
    return 0;
}

int isNumericColumn(char *column)
{
    return strcmp(column, "ID") == 0 || strcmp(column, "AGE") == 0 || strcmp(column, "MARKS") == 0 || strcmp(column, "DEPTID") == 0;
}

char *stripTablePrefix(char *column)
{
    char *dot = strchr(column, '.');
    if (dot != NULL)
    {
        return dot + 1;
    }
    return column;
}

void addColumn(Query *query, char *column)
{
    char cleaned[50];
    strcpy(cleaned, column);
    cleanToken(cleaned);
    normalizeToken(cleaned);

    if (strcmp(cleaned, "") == 0)
        return;

    strcpy(query->columns[query->columnCount], cleaned);
    query->columnCount++;
}

int isAggregateFunction(char *token, char *aggregate, char *column)
{
    char temp[50];
    strcpy(temp, token);
    cleanToken(temp);
    normalizeToken(temp);

    char *openParen = strchr(temp, '(');
    char *closeParen = strchr(temp, ')');
    if (openParen == NULL || closeParen == NULL || closeParen < openParen)
        return 0;

    *openParen = '\0';
    strcpy(aggregate, temp);

    char *value = openParen + 1;
    *closeParen = '\0';
    strcpy(column, value);

    if (strcmp(aggregate, "COUNT") == 0 || strcmp(aggregate, "AVG") == 0 || strcmp(aggregate, "MAX") == 0 || strcmp(aggregate, "MIN") == 0 || strcmp(aggregate, "SUM") == 0)
        return 1;

    return 0;
}

void parseJoinCondition(char *token, Query *query)
{
    char temp[50];
    strcpy(temp, token);
    cleanToken(temp);
    normalizeToken(temp);

    if (strcmp(temp, "=") == 0)
        return;

    char *equals = strchr(temp, '=');
    if (equals != NULL)
    {
        *equals = '\0';
        strcpy(query->leftJoinColumn, temp);
        strcpy(query->rightJoinColumn, equals + 1);
        return;
    }

    if (strcmp(query->leftJoinColumn, "") == 0)
    {
        strcpy(query->leftJoinColumn, temp);
    }
    else if (strcmp(query->rightJoinColumn, "") == 0)
    {
        strcpy(query->rightJoinColumn, temp);
    }
}

void addWhereCondition(Query *query, char *column, char *op, char *value)
{
    if (query->whereConditionCount >= 10)
        return;

    strcpy(query->whereConditions[query->whereConditionCount].column, column);
    strcpy(query->whereConditions[query->whereConditionCount].op, op);
    strcpy(query->whereConditions[query->whereConditionCount].value, value);

    if (query->whereConditionCount == 0)
    {
        strcpy(query->whereColumn, column);
        strcpy(query->whereOperator, op);
        strcpy(query->whereValue, value);
    }

    query->whereConditionCount++;
}

void parseQuery(char tokens[][50], int tokenCount, Query *query)
{
    strcpy(query->command, "");
    strcpy(query->table, "");
    query->hasFrom = 0;
    query->columnCount = 0;
    query->hasWhere = 0;
    strcpy(query->whereColumn, "");
    strcpy(query->whereOperator, "");
    strcpy(query->whereValue, "");
    query->whereConditionCount = 0;
    for (int i = 0; i < 10; i++)
    {
        strcpy(query->whereLogicalOps[i], "");
    }
    query->hasOrderBy = 0;
    strcpy(query->orderColumn, "");
    strcpy(query->orderType, "ASC");
    query->hasAggregate = 0;
    strcpy(query->aggregate, "");
    strcpy(query->aggregateColumn, "");
    query->hasJoin = 0;
    strcpy(query->joinTable, "");
    strcpy(query->leftJoinColumn, "");
    strcpy(query->rightJoinColumn, "");
    query->hasGroupBy = 0;
    strcpy(query->groupColumn, "");
    query->hasDistinct = 0;

    enum
    {
        READING_COLUMNS,
        READING_TABLE,
        READING_WHERE,
        READING_ORDERBY,
        READING_JOIN,
        READING_ON,
        READING_GROUPBY
    } mode = READING_COLUMNS;

    char currentColumn[20] = "";
    char currentOp[10] = "";
    char currentValue[20] = "";

    for (int i = 0; i < tokenCount; i++)
    {
        char cleaned[50];
        strcpy(cleaned, tokens[i]);
        cleanToken(cleaned);

        if (strcmp(cleaned, "") == 0)
            continue;

        char normalized[50];
        strcpy(normalized, cleaned);
        normalizeToken(normalized);

        if (i == 0)
        {
            strcpy(query->command, normalized);
        }
        else if (strcmp(normalized, "FROM") == 0)
        {
            query->hasFrom = 1;
            mode = READING_TABLE;
        }
        else if (strcmp(normalized, "WHERE") == 0)
        {
            mode = READING_WHERE;
            query->hasWhere = 1;
        }
        else if (strcmp(normalized, "ORDER") == 0)
        {
            mode = READING_ORDERBY;
            query->hasOrderBy = 1;
        }
        else if (strcmp(normalized, "GROUP") == 0)
        {
            mode = READING_GROUPBY;
            query->hasGroupBy = 1;
        }
        else if (strcmp(normalized, "DISTINCT") == 0)
        {
            query->hasDistinct = 1;
        }
        else if (strcmp(normalized, "JOIN") == 0)
        {
            mode = READING_JOIN;
            query->hasJoin = 1;
        }
        else if (strcmp(normalized, "ON") == 0)
        {
            mode = READING_ON;
        }
        else if (mode == READING_COLUMNS)
        {
            char aggregate[20];
            char aggregateColumn[20];
            if (isAggregateFunction(cleaned, aggregate, aggregateColumn))
            {
                query->hasAggregate = 1;
                strcpy(query->aggregate, aggregate);
                strcpy(query->aggregateColumn, aggregateColumn);
            }
            else
            {
                char *segment = strtok(cleaned, ",");
                while (segment != NULL)
                {
                    addColumn(query, segment);
                    segment = strtok(NULL, ",");
                }
            }
        }
        else if (mode == READING_TABLE)
        {
            normalizeToken(cleaned);
            strcpy(query->table, cleaned);
            mode = READING_COLUMNS;
        }
        else if (mode == READING_JOIN)
        {
            normalizeToken(cleaned);
            strcpy(query->joinTable, cleaned);
            mode = READING_COLUMNS;
        }
        else if (mode == READING_GROUPBY)
        {
            if (strcmp(normalized, "BY") == 0)
            {
                continue;
            }
            normalizeToken(cleaned);
            strcpy(query->groupColumn, cleaned);
            mode = READING_COLUMNS;
        }
        else if (mode == READING_ON)
        {
            parseJoinCondition(cleaned, query);
            if (strcmp(query->leftJoinColumn, "") != 0 && strcmp(query->rightJoinColumn, "") != 0)
            {
                mode = READING_COLUMNS;
            }
        }
        else if (mode == READING_WHERE)
        {
            if (strcmp(normalized, "AND") == 0 || strcmp(normalized, "OR") == 0)
            {
                if (strcmp(currentColumn, "") != 0 && strcmp(currentOp, "") != 0 && strcmp(currentValue, "") != 0)
                {
                    addWhereCondition(query, currentColumn, currentOp, currentValue);
                    if (query->whereConditionCount > 0)
                    {
                        strcpy(query->whereLogicalOps[query->whereConditionCount - 1], normalized);
                    }
                    strcpy(currentColumn, "");
                    strcpy(currentOp, "");
                    strcpy(currentValue, "");
                }
                continue;
            }

            if (strcmp(currentColumn, "") == 0)
            {
                normalizeToken(cleaned);
                strcpy(currentColumn, cleaned);
            }
            else if (strcmp(currentOp, "") == 0)
            {
                strcpy(currentOp, cleaned);
            }
            else if (strcmp(currentValue, "") == 0)
            {
                strcpy(currentValue, cleaned);
                addWhereCondition(query, currentColumn, currentOp, currentValue);
                strcpy(currentColumn, "");
                strcpy(currentOp, "");
                strcpy(currentValue, "");
            }
        }
        else if (mode == READING_ORDERBY)
        {
            if (strcmp(normalized, "BY") == 0)
            {
                continue;
            }
            if (strcmp(query->orderColumn, "") == 0)
            {
                normalizeToken(cleaned);
                strcpy(query->orderColumn, cleaned);
            }
            else if (strcmp(normalized, "ASC") == 0 || strcmp(normalized, "DESC") == 0)
            {
                strcpy(query->orderType, normalized);
            }
        }
    }
}

int validateQuery(Query *query, char *errorMessage, size_t size)
{
    if (strcmp(query->command, "SELECT") != 0)
    {
        if (strcmp(query->command, "SELCT") == 0)
        {
            snprintf(errorMessage, size, "ERROR: Unknown SQL keyword '%s'.\nDid you mean 'SELECT'?", query->command);
        }
        else
        {
            snprintf(errorMessage, size, "ERROR: Unknown SQL keyword '%s'.", query->command);
        }
        return 0;
    }

    if (!query->hasFrom)
    {
        snprintf(errorMessage, size, "ERROR: Expected keyword FROM.");
        return 0;
    }

    if (strcmp(query->table, "STUDENT") != 0 && strcmp(query->table, "DEPARTMENT") != 0)
    {
        snprintf(errorMessage, size, "ERROR: Table '%s' does not exist.", query->table);
        return 0;
    }

    if (!query->hasAggregate)
    {
        for (int i = 0; i < query->columnCount; i++)
        {
            if (strcmp(query->columns[i], "*") != 0 && !isAllowedColumn(query->columns[i]))
            {
                snprintf(errorMessage, size, "ERROR: Column '%s' does not exist in %s.", query->columns[i], query->table);
                return 0;
            }
        }
    }

    if (query->hasWhere)
    {
        int conditionCount = (query->whereConditionCount > 0) ? query->whereConditionCount : 1;
        for (int i = 0; i < conditionCount; i++)
        {
            char column[20];
            char op[10];
            char value[20];

            if (query->whereConditionCount > 0)
            {
                strcpy(column, query->whereConditions[i].column);
                strcpy(op, query->whereConditions[i].op);
                strcpy(value, query->whereConditions[i].value);
            }
            else
            {
                strcpy(column, query->whereColumn);
                strcpy(op, query->whereOperator);
                strcpy(value, query->whereValue);
            }

            if (!isAllowedColumn(column))
            {
                snprintf(errorMessage, size, "ERROR: Unknown column '%s'.", column);
                return 0;
            }

            if (strcmp(op, "=") != 0 && strcmp(op, "<") != 0 && strcmp(op, ">") != 0 && strcmp(op, ">=") != 0 && strcmp(op, "<=") != 0 && strcmp(op, "!=") != 0)
            {
                snprintf(errorMessage, size, "ERROR: Unsupported operator '%s'.", op);
                return 0;
            }
        }
    }

    if (query->hasAggregate)
    {
        if (strcmp(query->aggregate, "COUNT") != 0)
        {
            if (!isNumericColumn(query->aggregateColumn))
            {
                snprintf(errorMessage, size, "ERROR: %s() can only be used on numeric columns.", query->aggregate);
                return 0;
            }
        }
    }

    if (query->hasJoin)
    {
        char left[20];
        char right[20];
        strcpy(left, stripTablePrefix(query->leftJoinColumn));
        strcpy(right, stripTablePrefix(query->rightJoinColumn));
        normalizeToken(left);
        normalizeToken(right);
        if (strcmp(left, "DEPTID") != 0 || strcmp(right, "DEPTID") != 0)
        {
            snprintf(errorMessage, size, "ERROR: JOIN columns are incompatible.");
            return 0;
        }
    }

    if (query->hasGroupBy)
    {
        if (!isAllowedColumn(query->groupColumn))
        {
            snprintf(errorMessage, size, "ERROR: GROUP BY column '%s' not found.", query->groupColumn);
            return 0;
        }
    }

    return 1;
}

void printParsedQuery(Query *query)
{
    printf("\n----------- Parsed Query -----------\n");
    printf("Command : %s\n\n", query->command);

    if (query->hasAggregate)
    {
        printf("Aggregate : %s\n", query->aggregate);
        printf("Column : %s\n", query->aggregateColumn);
    }
    else
    {
        printf("Columns :\n");
        for (int i = 0; i < query->columnCount; i++)
        {
            printf("%s\n", query->columns[i]);
        }
    }

    printf("\nTable : %s\n", query->table);

    if (query->hasJoin)
    {
        printf("\nJOIN\n");
        printf("Table : %s\n", query->joinTable);
        printf("ON\n");
        printf("Left : %s\n", query->leftJoinColumn);
        printf("Right : %s\n", query->rightJoinColumn);
    }

    if (query->hasWhere)
    {
        printf("\nWHERE\n");
        int conditionCount = (query->whereConditionCount > 0) ? query->whereConditionCount : 1;
        for (int i = 0; i < conditionCount; i++)
        {
            char column[20];
            char op[10];
            char value[20];
            if (query->whereConditionCount > 0)
            {
                strcpy(column, query->whereConditions[i].column);
                strcpy(op, query->whereConditions[i].op);
                strcpy(value, query->whereConditions[i].value);
            }
            else
            {
                strcpy(column, query->whereColumn);
                strcpy(op, query->whereOperator);
                strcpy(value, query->whereValue);
            }
            printf("Condition %d : %s %s %s\n", i + 1, column, op, value);
            if (i < conditionCount - 1)
            {
                printf("Logical : %s\n", query->whereLogicalOps[i]);
            }
        }
    }

    if (query->hasOrderBy)
    {
        printf("\nORDER BY\n");
        printf("Column : %s\n", query->orderColumn);
        printf("Type : %s\n", query->orderType);
    }

    if (query->hasGroupBy)
    {
        printf("\nGROUP BY\n");
        printf("Column : %s\n", query->groupColumn);
    }

    if (query->hasDistinct)
    {
        printf("\nDISTINCT : YES\n");
    }
}

int compareStudents(Student *a, Student *b, Query *query)
{
    if (strcmp(query->orderColumn, "MARKS") == 0)
    {
        if (a->marks < b->marks)
            return -1;
        if (a->marks > b->marks)
            return 1;
    }
    else if (strcmp(query->orderColumn, "AGE") == 0)
    {
        if (a->age < b->age)
            return -1;
        if (a->age > b->age)
            return 1;
    }
    else if (strcmp(query->orderColumn, "NAME") == 0)
    {
        return strcmp(a->name, b->name);
    }
    else if (strcmp(query->orderColumn, "DEPTID") == 0)
    {
        if (a->deptID < b->deptID)
            return -1;
        if (a->deptID > b->deptID)
            return 1;
    }
    else if (strcmp(query->orderColumn, "ID") == 0)
    {
        if (a->id < b->id)
            return -1;
        if (a->id > b->id)
            return 1;
    }
    return 0;
}

void sortStudents(Student *students, int count, Query *query)
{
    for (int i = 0; i < count - 1; i++)
    {
        for (int j = 0; j < count - i - 1; j++)
        {
            int cmp = compareStudents(&students[j], &students[j + 1], query);
            int shouldSwap = 0;

            if (strcmp(query->orderType, "DESC") == 0)
            {
                shouldSwap = (cmp < 0);
            }
            else
            {
                shouldSwap = (cmp > 0);
            }

            if (shouldSwap)
            {
                Student temp = students[j];
                students[j] = students[j + 1];
                students[j + 1] = temp;
            }
        }
    }
}

int compareConditionValue(Student *student, WhereCondition *condition, int *result)
{
    int numericValue = atoi(condition->value);

    if (strcmp(condition->column, "AGE") == 0)
    {
        int actual = student->age;
        if (strcmp(condition->op, "=") == 0)
            *result = (actual == numericValue);
        else if (strcmp(condition->op, ">") == 0)
            *result = (actual > numericValue);
        else if (strcmp(condition->op, "<") == 0)
            *result = (actual < numericValue);
        else if (strcmp(condition->op, ">=") == 0)
            *result = (actual >= numericValue);
        else if (strcmp(condition->op, "<=") == 0)
            *result = (actual <= numericValue);
        else if (strcmp(condition->op, "!=") == 0)
            *result = (actual != numericValue);
        return 1;
    }
    else if (strcmp(condition->column, "MARKS") == 0)
    {
        int actual = student->marks;
        if (strcmp(condition->op, "=") == 0)
            *result = (actual == numericValue);
        else if (strcmp(condition->op, ">") == 0)
            *result = (actual > numericValue);
        else if (strcmp(condition->op, "<") == 0)
            *result = (actual < numericValue);
        else if (strcmp(condition->op, ">=") == 0)
            *result = (actual >= numericValue);
        else if (strcmp(condition->op, "<=") == 0)
            *result = (actual <= numericValue);
        else if (strcmp(condition->op, "!=") == 0)
            *result = (actual != numericValue);
        return 1;
    }
    else if (strcmp(condition->column, "DEPTID") == 0)
    {
        int actual = student->deptID;
        if (strcmp(condition->op, "=") == 0)
            *result = (actual == numericValue);
        else if (strcmp(condition->op, ">") == 0)
            *result = (actual > numericValue);
        else if (strcmp(condition->op, "<") == 0)
            *result = (actual < numericValue);
        else if (strcmp(condition->op, ">=") == 0)
            *result = (actual >= numericValue);
        else if (strcmp(condition->op, "<=") == 0)
            *result = (actual <= numericValue);
        else if (strcmp(condition->op, "!=") == 0)
            *result = (actual != numericValue);
        return 1;
    }
    else if (strcmp(condition->column, "ID") == 0)
    {
        int actual = student->id;
        if (strcmp(condition->op, "=") == 0)
            *result = (actual == numericValue);
        else if (strcmp(condition->op, ">") == 0)
            *result = (actual > numericValue);
        else if (strcmp(condition->op, "<") == 0)
            *result = (actual < numericValue);
        else if (strcmp(condition->op, ">=") == 0)
            *result = (actual >= numericValue);
        else if (strcmp(condition->op, "<=") == 0)
            *result = (actual <= numericValue);
        else if (strcmp(condition->op, "!=") == 0)
            *result = (actual != numericValue);
        return 1;
    }
    else if (strcmp(condition->column, "NAME") == 0)
    {
        if (strcmp(condition->op, "=") == 0)
            *result = (strcmp(student->name, condition->value) == 0);
        else if (strcmp(condition->op, "!=") == 0)
            *result = (strcmp(student->name, condition->value) != 0);
        return 1;
    }

    *result = 0;
    return 0;
}

void filterRows(ResultSet *input, Query *query, ResultSet *output)
{
    output->count = 0;

    for (int row = 0; row < input->count; row++)
    {
        int keepRow = 1;

        if (query->hasWhere)
        {
            int conditionCount = (query->whereConditionCount > 0) ? query->whereConditionCount : 1;
            for (int cond = 0; cond < conditionCount; cond++)
            {
                WhereCondition condition;
                if (query->whereConditionCount > 0)
                {
                    condition = query->whereConditions[cond];
                }
                else
                {
                    strcpy(condition.column, query->whereColumn);
                    strcpy(condition.op, query->whereOperator);
                    strcpy(condition.value, query->whereValue);
                }

                int conditionResult = 0;
                compareConditionValue(&input->rows[row], &condition, &conditionResult);

                if (cond == 0)
                {
                    keepRow = conditionResult;
                }
                else if (strcmp(query->whereLogicalOps[cond - 1], "AND") == 0)
                {
                    keepRow = keepRow && conditionResult;
                }
                else if (strcmp(query->whereLogicalOps[cond - 1], "OR") == 0)
                {
                    keepRow = keepRow || conditionResult;
                }
            }
        }

        if (keepRow)
        {
            output->rows[output->count] = input->rows[row];
            output->count++;
        }
    }
}

int valueExists(ResultSet *result, Query *query, int columnIndex, char *value)
{
    for (int i = 0; i < result->count; i++)
    {
        if (strcmp(query->columns[columnIndex], "ID") == 0)
        {
            char current[20];
            sprintf(current, "%d", result->rows[i].id);
            if (strcmp(current, value) == 0)
                return 1;
        }
        else if (strcmp(query->columns[columnIndex], "NAME") == 0)
        {
            if (strcmp(result->rows[i].name, value) == 0)
                return 1;
        }
        else if (strcmp(query->columns[columnIndex], "AGE") == 0)
        {
            char current[20];
            sprintf(current, "%d", result->rows[i].age);
            if (strcmp(current, value) == 0)
                return 1;
        }
        else if (strcmp(query->columns[columnIndex], "MARKS") == 0)
        {
            char current[20];
            sprintf(current, "%d", result->rows[i].marks);
            if (strcmp(current, value) == 0)
                return 1;
        }
        else if (strcmp(query->columns[columnIndex], "DEPTID") == 0)
        {
            char current[20];
            sprintf(current, "%d", result->rows[i].deptID);
            if (strcmp(current, value) == 0)
                return 1;
        }
    }
    return 0;
}

void buildRowSignature(ResultSet *result, Query *query, int rowIndex, int selectedColumns, int isWildcard, char *signature, size_t size)
{
    signature[0] = '\0';
    for (int col = 0; col < selectedColumns; col++)
    {
        char value[20];
        if (isWildcard)
        {
            switch (col)
            {
            case 0:
                sprintf(value, "%d", result->rows[rowIndex].id);
                break;
            case 1:
                strcpy(value, result->rows[rowIndex].name);
                break;
            case 2:
                sprintf(value, "%d", result->rows[rowIndex].age);
                break;
            case 3:
                sprintf(value, "%d", result->rows[rowIndex].marks);
                break;
            case 4:
                sprintf(value, "%d", result->rows[rowIndex].deptID);
                break;
            }
        }
        else
        {
            if (strcmp(query->columns[col], "ID") == 0)
                sprintf(value, "%d", result->rows[rowIndex].id);
            else if (strcmp(query->columns[col], "NAME") == 0)
                strcpy(value, result->rows[rowIndex].name);
            else if (strcmp(query->columns[col], "AGE") == 0)
                sprintf(value, "%d", result->rows[rowIndex].age);
            else if (strcmp(query->columns[col], "MARKS") == 0)
                sprintf(value, "%d", result->rows[rowIndex].marks);
            else if (strcmp(query->columns[col], "DEPTID") == 0)
                sprintf(value, "%d", result->rows[rowIndex].deptID);
        }

        if (strlen(signature) > 0)
        {
            strncat(signature, "|", size - strlen(signature) - 1);
        }
        strncat(signature, value, size - strlen(signature) - 1);
    }
}

void printResultSet(ResultSet *result, Query *query)
{
    if (result->count == 0)
    {
        printf("No rows found.\n");
        return;
    }

    int selectedColumns = query->columnCount;
    int isWildcard = (selectedColumns == 1 && strcmp(query->columns[0], "*") == 0);

    if (isWildcard)
    {
        selectedColumns = 5;
    }

    char headers[5][20] = {"ID", "Name", "Age", "Marks", "DeptID"};
    int widths[5] = {2, 6, 3, 5, 6};

    if (!isWildcard)
    {
        for (int i = 0; i < query->columnCount; i++)
        {
            if (strcmp(query->columns[i], "ID") == 0)
            {
                widths[i] = 2;
            }
            else if (strcmp(query->columns[i], "NAME") == 0)
            {
                widths[i] = 6;
            }
            else if (strcmp(query->columns[i], "AGE") == 0)
            {
                widths[i] = 3;
            }
            else if (strcmp(query->columns[i], "MARKS") == 0)
            {
                widths[i] = 5;
            }
            else if (strcmp(query->columns[i], "DEPTID") == 0)
            {
                widths[i] = 6;
            }
        }
    }

    printf("\n");
    for (int i = 0; i < selectedColumns; i++)
    {
        printf("+");
        for (int j = 0; j < widths[i] + 2; j++)
            printf("-");
    }
    printf("+\n");

    printf("|");
    for (int i = 0; i < selectedColumns; i++)
    {
        char header[20];
        if (isWildcard)
        {
            strcpy(header, headers[i]);
        }
        else
        {
            strcpy(header, query->columns[i]);
        }

        printf(" %-*s |", widths[i], header);
    }
    printf("\n");

    for (int i = 0; i < selectedColumns; i++)
    {
        printf("+");
        for (int j = 0; j < widths[i] + 2; j++)
            printf("-");
    }
    printf("+\n");

    char seenSignatures[100][200];
    int seenCount = 0;

    for (int row = 0; row < result->count; row++)
    {
        char signature[200];
        buildRowSignature(result, query, row, selectedColumns, isWildcard, signature, sizeof(signature));

        int isDuplicate = 0;
        if (query->hasDistinct)
        {
            for (int seen = 0; seen < seenCount; seen++)
            {
                if (strcmp(seenSignatures[seen], signature) == 0)
                {
                    isDuplicate = 1;
                    break;
                }
            }
        }

        if (isDuplicate)
            continue;

        if (query->hasDistinct)
        {
            strcpy(seenSignatures[seenCount], signature);
            seenCount++;
        }

        printf("|");
        for (int col = 0; col < selectedColumns; col++)
        {
            if (isWildcard)
            {
                switch (col)
                {
                case 0:
                    printf(" %-2d |", result->rows[row].id);
                    break;
                case 1:
                    printf(" %-6s |", result->rows[row].name);
                    break;
                case 2:
                    printf(" %-3d |", result->rows[row].age);
                    break;
                case 3:
                    printf(" %-5d |", result->rows[row].marks);
                    break;
                case 4:
                    printf(" %-6d |", result->rows[row].deptID);
                    break;
                }
            }
            else
            {
                if (strcmp(query->columns[col], "ID") == 0)
                    printf(" %-2d |", result->rows[row].id);
                else if (strcmp(query->columns[col], "NAME") == 0)
                    printf(" %-6s |", result->rows[row].name);
                else if (strcmp(query->columns[col], "AGE") == 0)
                    printf(" %-3d |", result->rows[row].age);
                else if (strcmp(query->columns[col], "MARKS") == 0)
                    printf(" %-5d |", result->rows[row].marks);
                else if (strcmp(query->columns[col], "DEPTID") == 0)
                    printf(" %-6d |", result->rows[row].deptID);
            }
        }
        printf("\n");
    }

    for (int i = 0; i < selectedColumns; i++)
    {
        printf("+");
        for (int j = 0; j < widths[i] + 2; j++)
            printf("-");
    }
    printf("+\n");
}

int getNumericValue(Student *student, char *column)
{
    if (strcmp(column, "ID") == 0)
        return student->id;
    if (strcmp(column, "AGE") == 0)
        return student->age;
    if (strcmp(column, "MARKS") == 0)
        return student->marks;
    if (strcmp(column, "DEPTID") == 0)
        return student->deptID;
    return 0;
}

void printAggregateResult(Query *query, double value)
{
    char header[20];
    strcpy(header, query->aggregate);

    char valueText[20];
    if (strcmp(query->aggregate, "AVG") == 0)
    {
        sprintf(valueText, "%.1f", value);
    }
    else
    {
        sprintf(valueText, "%.0f", value);
    }

    int width = strlen(header);
    int valueWidth = strlen(valueText);
    if (valueWidth > width)
        width = valueWidth;

    printf("\n");
    printf("+");
    for (int i = 0; i < width + 2; i++)
        printf("-");
    printf("+\n");

    printf("| %-*s |\n", width, header);

    printf("+");
    for (int i = 0; i < width + 2; i++)
        printf("-");
    printf("+\n");

    printf("| %-*s |\n", width, valueText);

    printf("+");
    for (int i = 0; i < width + 2; i++)
        printf("-");
    printf("+\n");
}

double computeAggregateValue(ResultSet *input, Query *query)
{
    if (strcmp(query->aggregate, "COUNT") == 0)
    {
        return (double)input->count;
    }

    if (input->count == 0)
    {
        return 0.0;
    }

    double result = 0.0;

    if (strcmp(query->aggregate, "MAX") == 0)
    {
        int currentValue = getNumericValue(&input->rows[0], query->aggregateColumn);
        for (int i = 1; i < input->count; i++)
        {
            int candidate = getNumericValue(&input->rows[i], query->aggregateColumn);
            if (candidate > currentValue)
                currentValue = candidate;
        }
        return (double)currentValue;
    }

    if (strcmp(query->aggregate, "MIN") == 0)
    {
        int currentValue = getNumericValue(&input->rows[0], query->aggregateColumn);
        for (int i = 1; i < input->count; i++)
        {
            int candidate = getNumericValue(&input->rows[i], query->aggregateColumn);
            if (candidate < currentValue)
                currentValue = candidate;
        }
        return (double)currentValue;
    }

    for (int i = 0; i < input->count; i++)
    {
        result += getNumericValue(&input->rows[i], query->aggregateColumn);
    }

    if (strcmp(query->aggregate, "SUM") == 0)
    {
        return result;
    }
    if (strcmp(query->aggregate, "AVG") == 0)
    {
        return result / input->count;
    }

    return 0.0;
}

void executeAggregateQuery(ResultSet *input, Query *query)
{
    printAggregateResult(query, computeAggregateValue(input, query));
}

void executeGroupByQuery(ResultSet *input, Query *query)
{
    if (input->count == 0)
    {
        printf("No rows found.\n");
        return;
    }

    Group groups[100];
    int groupCount = 0;

    for (int row = 0; row < input->count; row++)
    {
        int groupValue = getNumericValue(&input->rows[row], query->groupColumn);
        int found = -1;

        for (int g = 0; g < groupCount; g++)
        {
            if (groups[g].key == groupValue)
            {
                found = g;
                break;
            }
        }

        if (found == -1)
        {
            groups[groupCount].key = groupValue;
            groups[groupCount].rowCount = 0;
            groups[groupCount].rows[groups[groupCount].rowCount] = input->rows[row];
            groups[groupCount].rowCount++;
            groupCount++;
        }
        else
        {
            groups[found].rows[groups[found].rowCount] = input->rows[row];
            groups[found].rowCount++;
        }
    }

    char headers[2][20] = {"", ""};
    strcpy(headers[0], query->columns[0]);
    strcpy(headers[1], query->aggregate);

    int widths[2] = {strlen(headers[0]), strlen(headers[1])};

    printf("\n");
    for (int i = 0; i < 2; i++)
    {
        printf("+");
        for (int j = 0; j < widths[i] + 2; j++)
            printf("-");
    }
    printf("+\n");

    printf("|");
    for (int i = 0; i < 2; i++)
    {
        printf(" %-*s |", widths[i], headers[i]);
    }
    printf("\n");

    for (int i = 0; i < 2; i++)
    {
        printf("+");
        for (int j = 0; j < widths[i] + 2; j++)
            printf("-");
    }
    printf("+\n");

    for (int g = 0; g < groupCount; g++)
    {
        ResultSet groupRows;
        groupRows.count = groups[g].rowCount;
        for (int i = 0; i < groups[g].rowCount; i++)
        {
            groupRows.rows[i] = groups[g].rows[i];
        }

        double aggValue = computeAggregateValue(&groupRows, query);

        printf("|");
        printf(" %-*d |", widths[0], groups[g].key);
        if (strcmp(query->aggregate, "AVG") == 0)
        {
            printf(" %-*.1f |", widths[1], aggValue);
        }
        else
        {
            printf(" %-*.0f |", widths[1], aggValue);
        }
        printf("\n");
    }

    for (int i = 0; i < 2; i++)
    {
        printf("+");
        for (int j = 0; j < widths[i] + 2; j++)
            printf("-");
    }
    printf("+\n");
}

int compareJoinedRows(JoinedRow *a, JoinedRow *b, Query *query)
{
    if (strcmp(query->orderColumn, "NAME") == 0)
    {
        return strcmp(a->student.name, b->student.name);
    }
    else if (strcmp(query->orderColumn, "DEPARTMENT") == 0)
    {
        return strcmp(a->department.department, b->department.department);
    }
    else if (strcmp(query->orderColumn, "AGE") == 0)
    {
        if (a->student.age < b->student.age)
            return -1;
        if (a->student.age > b->student.age)
            return 1;
    }
    else if (strcmp(query->orderColumn, "MARKS") == 0)
    {
        if (a->student.marks < b->student.marks)
            return -1;
        if (a->student.marks > b->student.marks)
            return 1;
    }
    else if (strcmp(query->orderColumn, "DEPTID") == 0)
    {
        if (a->student.deptID < b->student.deptID)
            return -1;
        if (a->student.deptID > b->student.deptID)
            return 1;
    }
    else if (strcmp(query->orderColumn, "ID") == 0)
    {
        if (a->student.id < b->student.id)
            return -1;
        if (a->student.id > b->student.id)
            return 1;
    }
    return 0;
}

void sortJoinedRows(JoinedRow *rows, int count, Query *query)
{
    for (int i = 0; i < count - 1; i++)
    {
        for (int j = 0; j < count - i - 1; j++)
        {
            int cmp = compareJoinedRows(&rows[j], &rows[j + 1], query);
            int shouldSwap = 0;
            if (strcmp(query->orderType, "DESC") == 0)
            {
                shouldSwap = (cmp < 0);
            }
            else
            {
                shouldSwap = (cmp > 0);
            }
            if (shouldSwap)
            {
                JoinedRow temp = rows[j];
                rows[j] = rows[j + 1];
                rows[j + 1] = temp;
            }
        }
    }
}

void printJoinedResultSet(JoinedResultSet *result, Query *query)
{
    if (result->count == 0)
    {
        printf("No rows found.\n");
        return;
    }

    int selectedColumns = query->columnCount;
    int widths[20] = {0};

    for (int i = 0; i < selectedColumns; i++)
    {
        widths[i] = strlen(query->columns[i]);
        if (strcmp(query->columns[i], "NAME") == 0)
            widths[i] = 6;
        else if (strcmp(query->columns[i], "DEPARTMENT") == 0)
            widths[i] = 11;
        else if (strcmp(query->columns[i], "ID") == 0)
            widths[i] = 2;
        else if (strcmp(query->columns[i], "AGE") == 0)
            widths[i] = 3;
        else if (strcmp(query->columns[i], "MARKS") == 0)
            widths[i] = 5;
        else if (strcmp(query->columns[i], "DEPTID") == 0)
            widths[i] = 6;
    }

    printf("\n");
    for (int i = 0; i < selectedColumns; i++)
    {
        printf("+");
        for (int j = 0; j < widths[i] + 2; j++)
            printf("-");
    }
    printf("+\n");

    printf("|");
    for (int i = 0; i < selectedColumns; i++)
    {
        printf(" %-*s |", widths[i], query->columns[i]);
    }
    printf("\n");

    for (int i = 0; i < selectedColumns; i++)
    {
        printf("+");
        for (int j = 0; j < widths[i] + 2; j++)
            printf("-");
    }
    printf("+\n");

    for (int row = 0; row < result->count; row++)
    {
        printf("|");
        for (int col = 0; col < selectedColumns; col++)
        {
            if (strcmp(query->columns[col], "NAME") == 0)
                printf(" %-*s |", widths[col], result->rows[row].student.name);
            else if (strcmp(query->columns[col], "DEPARTMENT") == 0)
                printf(" %-*s |", widths[col], result->rows[row].department.department);
            else if (strcmp(query->columns[col], "ID") == 0)
                printf(" %-*d |", widths[col], result->rows[row].student.id);
            else if (strcmp(query->columns[col], "AGE") == 0)
                printf(" %-*d |", widths[col], result->rows[row].student.age);
            else if (strcmp(query->columns[col], "MARKS") == 0)
                printf(" %-*d |", widths[col], result->rows[row].student.marks);
            else if (strcmp(query->columns[col], "DEPTID") == 0)
                printf(" %-*d |", widths[col], result->rows[row].student.deptID);
        }
        printf("\n");
    }

    for (int i = 0; i < selectedColumns; i++)
    {
        printf("+");
        for (int j = 0; j < widths[i] + 2; j++)
            printf("-");
    }
    printf("+\n");
}

void executeJoinQuery(Query *query, Student *students, int count, Department *departments, int deptCount)
{
    JoinedResultSet joined;
    joined.count = 0;

    for (int i = 0; i < count; i++)
    {
        for (int j = 0; j < deptCount; j++)
        {
            if (students[i].deptID == departments[j].deptID)
            {
                joined.rows[joined.count].student = students[i];
                joined.rows[joined.count].department = departments[j];
                joined.count++;
            }
        }
    }

    if (query->hasOrderBy)
    {
        sortJoinedRows(joined.rows, joined.count, query);
    }

    printJoinedResultSet(&joined, query);
}

void executeQuery(Query *query, Student *students, int count, Department *departments, int deptCount)
{
    if (strcmp(query->command, "SELECT") == 0 && strcmp(query->table, "STUDENT") == 0)
    {
        if (query->hasJoin)
        {
            executeJoinQuery(query, students, count, departments, deptCount);
            return;
        }

        ResultSet input;
        input.count = count;
        for (int i = 0; i < count; i++)
        {
            input.rows[i] = students[i];
        }

        ResultSet working;
        ResultSet filtered;

        if (query->hasWhere)
        {
            filterRows(&input, query, &filtered);
            working = filtered;
        }
        else
        {
            working = input;
        }

        if (query->hasGroupBy)
        {
            executeGroupByQuery(&working, query);
        }
        else if (query->hasAggregate)
        {
            executeAggregateQuery(&working, query);
        }
        else
        {
            if (query->hasOrderBy)
            {
                sortStudents(working.rows, working.count, query);
            }

            printResultSet(&working, query);
        }
    }
}

int main()
{
    Student students[100];
    int count = 0;
    Department departments[100];
    int deptCount = 0;

    FILE *fp = fopen("student.csv", "r");
    if (fp == NULL)
    {
        printf("Error opening file\n");
        return 1;
    }

    char line[100];

    fgets(line, sizeof(line), fp);

    while (fgets(line, sizeof(line), fp))
    {
        char *token = strtok(line, ",");
        if (token == NULL)
            continue;

        students[count].id = atoi(token);

        token = strtok(NULL, ",");
        if (token != NULL)
        {
            strcpy(students[count].name, token);
            students[count].name[strcspn(students[count].name, "\n")] = '\0';
        }

        token = strtok(NULL, ",");
        if (token != NULL)
            students[count].age = atoi(token);

        token = strtok(NULL, ",");
        if (token != NULL)
            students[count].marks = atoi(token);

        token = strtok(NULL, ",");
        if (token != NULL)
            students[count].deptID = atoi(token);

        count++;
    }

    fclose(fp);

    fp = fopen("department.csv", "r");
    if (fp == NULL)
    {
        printf("Error opening department file\n");
        return 1;
    }

    fgets(line, sizeof(line), fp);

    while (fgets(line, sizeof(line), fp))
    {
        char *token = strtok(line, ",");
        if (token == NULL)
            continue;

        departments[deptCount].deptID = atoi(token);

        token = strtok(NULL, ",");
        if (token != NULL)
        {
            strcpy(departments[deptCount].department, token);
            departments[deptCount].department[strcspn(departments[deptCount].department, "\n")] = '\0';
        }

        deptCount++;
    }

    fclose(fp);

    printf("Mini DBMS Started\n\n");

    char query[200];
    printf("SQL> ");
    if (fgets(query, sizeof(query), stdin) == NULL)
    {
        printf("No query entered.\n");
        return 1;
    }

    query[strcspn(query, "\n")] = '\0';

    char tokens[100][50];
    int tokenCount = 0;

    tokenizeQuery(query, tokens, &tokenCount);

    Query parsedQuery;
    parseQuery(tokens, tokenCount, &parsedQuery);

    char errorMessage[200];
    if (!validateQuery(&parsedQuery, errorMessage, sizeof(errorMessage)))
    {
        printf("%s\n", errorMessage);
        return 0;
    }

    printParsedQuery(&parsedQuery);
    executeQuery(&parsedQuery, students, count, departments, deptCount);

    return 0;
}

