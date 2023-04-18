// Neural Network serial code, v2.0
// Anu, Thomas, Zack

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "evaluation.h"
#include <omp.h>


#define learningRate 0.0001f                    // defining a constant for learning rate
#define numberOfEpochs 5                        // number of epochs 1500

//double sigmoid(double x) { return 1/(1+exp(-x)); } //forward propagation
double dSigmoid(double x) {
    return sigmoid(x) * (1 - sigmoid(x));      // derivative of sigmoid for backpropagation
}

//double relu(double x) { return MAX(x,0) ;}
double dRelu(double x) {
    if(x<0)
    {
        return 0;                              // derivative of ReLU for backpropagation
    }
    else
    {
        return 1;
    }
}

double initWeights() {
    return ((double)rand()) / ((double)RAND_MAX);  // function to initialize weights
}

// random shuffle data
void shuffle(int *array, size_t n){
    // Initializes random number generator
    srand(44);

    if (n > 1){
        size_t i;
        for(i = 0; i < n-1; i++){
            size_t j = i + rand() / (RAND_MAX / (n - i) + 1);  // generate a random index to swap with
            int temp = array[j];                               // creating a temporary variable
            array[j] = array[i];                               // updating the values
            array[i] = temp;                                   // swapping the elements
        }
    }
}

#define numInputs 2400               // number of columns
#define numHiddenNodes 2400          // number of nodes in the first hidden layer
#define numHiddenNodes2 2400         // number of nodes in the second hidden layer
#define numOutputs 1               // number of outputs
#define numTrain 455
#define numTest 114
#define numTrainingSets 569        // number of instances of total data

int main() {

    // learning rate
    const double lr = learningRate;
    int thread_count = 64;

    // Declare pointers for dynamically allocated arrays
    double* hiddenLayer;
    double* hiddenLayer2;
    double* outputLayer;
    double* hiddenLayerBias;
    double* hiddenLayerBias2;
    double* outputLayerBias;
    double** hiddenWeights;
    double** hiddenWeights2;
    double** outputWeights;

    // Allocate memory for hidden layer nodes vectors
    hiddenLayer = (double*) malloc(numHiddenNodes * sizeof(double));
    hiddenLayer2 = (double*) malloc(numHiddenNodes2 * sizeof(double));
    outputLayer = (double*) malloc(numOutputs * sizeof(double));

    // Allocate memory for hidden layer bias vectors
    hiddenLayerBias = (double*) malloc(numHiddenNodes * sizeof(double));
    hiddenLayerBias2 = (double*) malloc(numHiddenNodes2 * sizeof(double));
    outputLayerBias = (double*) malloc(numOutputs * sizeof(double));

    // Allocate memory for hidden and output weights matrices
    hiddenWeights = (double**) malloc(numInputs * sizeof(double*));
    hiddenWeights2 = (double**) malloc(numInputs * sizeof(double*));
    outputWeights = (double**) malloc(numHiddenNodes2 * sizeof(double*));

    for (int i = 0; i < numInputs; i++) {
        hiddenWeights[i] = (double*) malloc(numHiddenNodes * sizeof(double));
        hiddenWeights2[i] = (double*) malloc(numHiddenNodes2 * sizeof(double));
    }

    for (int i = 0; i < numHiddenNodes2; i++) {
        outputWeights[i] = (double*) malloc(numOutputs * sizeof(double));
    }

    // Dynamically allocate memory for training and testing inputs and outputs
    double **trainingInputs = (double **)malloc(numTrain * sizeof(double *));
    for (int i = 0; i < numTrain; i++) {
        trainingInputs[i] = (double *)malloc(numInputs * sizeof(double));
    }

    double **testingInputs = (double **)malloc(numTest * sizeof(double *));
    for (int i = 0; i < numTest; i++) {
        testingInputs[i] = (double *)malloc(numInputs * sizeof(double));
    }

    double **trainingOutputs = (double **)malloc(numTrain * sizeof(double *));
    for (int i = 0; i < numTrain; i++) {
        trainingOutputs[i] = (double *)malloc(numOutputs * sizeof(double));
    }

    double *testingOutputs = (double *)malloc(numTest * sizeof(double));



    // read data from inputTrain.csv
    char buffer[802400];
    char buffer2[802400];
    char *record, *line;
    char *record2, *line2;
    int i = 0, j = 0;
    double inputTrain[numTrain][2401];
    double inputTest[numTrain][2401];

    // read input data from a csv
    FILE *fstream = fopen("train_data2401.csv", "r");
    if (fstream == NULL) {
        printf("\n file opening failed train ");
        return -1;
    }
    while ((line = fgets(buffer, sizeof(buffer), fstream)) != NULL) {
        record = strtok(line, ",");
        while (record != NULL) {
            inputTrain[i][j++] = strtod(record, NULL);
            record = strtok(NULL, ",");
        }
        if (j == 2400)
            i += 1;
    }

    fclose(fstream);

    i = 0, j = 0;

    // read data from inputTrain.csv
    // read input data from a csv

    FILE *gstream = fopen("test_data2401.csv", "r");
    if (gstream == NULL) {
        printf("\n file opening failed test ");
        return -1;
    }
    while ((line2 = fgets(buffer2, sizeof(buffer2), gstream)) != NULL) {
        record2 = strtok(line2, ",");
        //printf("%s ", record2);
        while (record2 != NULL) {
            inputTest[i][j++] = strtod(record2, NULL);
            record2 = strtok(NULL, ",");
        }
        if (j == 2400)
            i += 1;
    }

    // training data (inputs)
    // double trainingInputs[numTrain][numInputs];

    #pragma omp parallel for num_threads(thread_count) collapse(2) shared(trainingInputs, inputTrain)
    for (int ro=0; ro<numTrain; ro++)
    {
        for(int columns=1; columns<2401; columns++)
        {
            trainingInputs[ro][columns-1] = inputTrain[ro][columns];
        }
    }

    // testing data (inputs)
    // double testingInputs[numTest][numInputs];

    #pragma omp parallel for num_threads(thread_count) shared(testingInputs, inputTest)
    for (int ro=0; ro<numTest; ro++)
    {
        for(int columns=1; columns<2401; columns++)
        {
            //int rowX = newTestingSetOrder[ro];
            testingInputs[ro][columns-1] = inputTest[ro][columns];
            //printf("%f ", testingInputs[ro][columns-1]);

        }
    }

    // training data (outputs)
    // double trainingOutputs[numTrain][numOutputs];

    #pragma omp parallel for num_threads(thread_count) shared(trainingOutputs, inputTrain)
    for (int ro=0; ro<numTrain; ro++)
    {
        for(int columns=0; columns<1; columns++)
        {
            //int row = newTrainingSetOrder[ro];
            trainingOutputs[ro][columns] = inputTrain[ro][columns];
        }
    }

    // testing data (outputs)
    // double testingOutputs[numTest];

    #pragma omp parallel for num_threads(thread_count) shared(testingOutputs, inputTest)
    for (int ro=0; ro<numTest; ro++)
    {
        for(int columns=0; columns<1; columns++)
        {
            //int row = newTestingSetOrder[ro];
            testingOutputs[ro] = inputTest[ro][columns];
        }
    }

    // initialize bias and weight terms to random
    // hidden layer 1 weights
    for(int i = 0; i < numHiddenNodes; i++){
        hiddenLayerBias[i] = initWeights();
    }
    // hidden layer 2 weights
    for(int i = 0; i < numHiddenNodes2; i++){
        hiddenLayerBias2[i] = initWeights();
    }
    // output layer weights
    for(int i = 0; i < numOutputs; i++){
        outputLayerBias[i] = initWeights();
    }
    // hidden layer 1 bias
    for(int i = 0; i < numInputs; i++){
        for(int j = 0; j < numHiddenNodes; j++){
            hiddenWeights[i][j] = initWeights();
        }
    }
    // hidden layer 2 bias
    for(int i = 0; i < numHiddenNodes; i++){
        for(int j = 0; j < numHiddenNodes2; j++){
            hiddenWeights2[i][j] = initWeights();
        }
    }
    // output layer bias
    for(int i = 0 ; i < numHiddenNodes; i++){
        for(int j = 0; j < numOutputs; j++){
            outputWeights[i][j] = initWeights();
        }
    }

    // specify training set
    int trainingSetOrder[numTrain];
    for(int i = 0 ; i < numTrain ; i++)
    {
        trainingSetOrder[i] = i;
    }

    // shuffling training set
    // shuffle(trainingSetOrder, cutOffTrain);


    // int numberOfEpochs = 1500;                            // number of epochs

    // start time measurement
    clock_t start, end;
    start = clock();
    //training loop
    for(int epoch = 0; epoch < numberOfEpochs; epoch++){

        shuffle(trainingSetOrder, numTrain);

        for(int x = 0; x < numTrain; x ++){
            int i = trainingSetOrder[x];

            // forward pass
            // compute hidden layer activation

            // hidden layer 1
            int k;
            #pragma omp parallel for num_threads(thread_count) shared(trainingInputs, hiddenWeights, hiddenLayer, hiddenLayerBias) private(j, k)
            for(int j =0; j < numHiddenNodes; j++){
                double activation = hiddenLayerBias[j];

                for(int k = 0; k < numInputs; k++){
                    activation += trainingInputs[i][k] * hiddenWeights[k][j];
                }

                hiddenLayer[j] = relu(activation);
            }

            // hidden layer 2
            #pragma omp parallel for num_threads(thread_count) shared(hiddenLayer, hiddenWeights2, hiddenLayer2, hiddenLayerBias2) private(j, k)
            for(int j =0; j < numHiddenNodes2; j++){
                double activation = hiddenLayerBias2[j];

                for(int k = 0; k < numHiddenNodes; k++){
                    activation += hiddenLayer[k] * hiddenWeights2[k][j];
                }

                hiddenLayer2[j] = relu(activation);
            }

            // compute output layer activation
            #pragma omp parallel for num_threads(thread_count) shared(hiddenLayer2, outputWeights, outputLayer, outputLayerBias) private(j, k)
            for(int j =0; j < numOutputs; j++){
                double activation = outputLayerBias[j];

                for(int k = 0; k < numHiddenNodes2; k++){
                    activation += hiddenLayer2[k] * outputWeights[k][j];
                }

                outputLayer[j] = sigmoid(activation);
            }

            printf("Input: %g | %g | %g | %g | %g | %g |      Output: %g      Expected Output: %g \n",
                   trainingInputs[i][1], trainingInputs[i][2], trainingInputs[i][3], trainingInputs[i][4], trainingInputs[i][5], trainingInputs[i][6],
                   outputLayer[0], trainingOutputs[i][0]);

            // Backpropagation
            // Compute change in output weights
            double deltaOutput[numOutputs];
            #pragma omp parallel for num_threads(thread_count) shared(deltaOutput, trainingOutputs, outputLayer) private(j)
            for(int j = 0; j < numOutputs; j++){
                double error = (trainingOutputs[i][j] - outputLayer[j]); // L1
                //double error = -((outputLayer[j] * log(trainingOutputs[i][j])) + ((1-outputLayer[j]) * log(trainingOutputs[i][j]))); // cross entropy
                //double error = (1/numOutputs)*(pow(trainingOutputs[i][j] - outputLayer[j],2)); // MSE
                deltaOutput[j] = error * dSigmoid(outputLayer[j]) ;
            }

            // Compute change in hidden weights (second layer)
            double deltaHidden2[numHiddenNodes2];
            #pragma omp parallel for num_threads(thread_count) shared(deltaHidden2, deltaOutput, outputWeights, hiddenLayer, hiddenLayer2) private(j, k)
            for(int j = 0; j < numHiddenNodes2; j++){
                double error = 0.0f;
                for(int k = 0; k < numOutputs; k++){
                    error += deltaOutput[k] * outputWeights[j][k];
                }
                deltaHidden2[j] = error * dRelu(hiddenLayer[j]);
            }

            // Compute change in hidden weights (first layer)
            double deltaHidden[numHiddenNodes];
            #pragma omp parallel for num_threads(thread_count) shared(deltaHidden, deltaHidden2, hiddenWeights2, hiddenLayer, hiddenLayerBias2) private(j, k)
            for(int j = 0; j < numHiddenNodes; j++){
                double error = 0.0f;
                for(int k = 0; k < numHiddenNodes2; k++){
                    error += deltaHidden2[k] * hiddenWeights2[j][k];
                }
                deltaHidden[j] = error * dRelu(hiddenLayer2[j]);
            }

            // Apply change in output weights
            #pragma omp parallel for num_threads(thread_count) shared(outputLayerBias, outputWeights, deltaOutput) private(j, k)
            for(int j = 0; j < numOutputs; j++){
                outputLayerBias[j] += deltaOutput[j] * lr;
                for(int k = 0; k < numHiddenNodes2; k++){
                    outputWeights[k][j] += hiddenLayer2[k] * deltaOutput[j] * lr;
                }
            }

            // Apply change in second hidden layer weights
            #pragma omp parallel for num_threads(thread_count) shared(hiddenLayerBias, hiddenWeights2, deltaHidden) private(j, k)
            for(int j = 0; j < numHiddenNodes2; j++){
                hiddenLayerBias[j] += deltaHidden[j] * lr;
                for(int k = 0; k < numHiddenNodes; k++){
                    hiddenWeights2[k][j] += hiddenLayer[k] * deltaHidden2[j] * lr;
                }
            }

            // Apply change in first hidden layer weights
            #pragma omp parallel for num_threads(thread_count) shared(hiddenLayerBias, hiddenWeights, deltaHidden) private(j, k)
            for(int j = 0; j < numHiddenNodes; j++){
                hiddenLayerBias[j] += deltaHidden[j] * lr;
                for(int k = 0; k < numInputs; k++){
                    hiddenWeights[k][j] += trainingInputs[i][k] * deltaHidden[j] * lr;
                }
            }
        }
    }

    end = clock();
/*
    // print final weights after done training
    fputs ("\nFinal Hidden Weights\n[ ", stdout);
    for(int j = 0; j < numHiddenNodes; j++){
        fputs ("[ ", stdout);
        for(int k = 0; k < numInputs; k++){
            printf("%f ", hiddenWeights[k][j]);
        }
        fputs("] ", stdout);
    }
    fputs ("\nFinal Hidden2 Weights\n[ ", stdout);
    for(int j = 0; j < numHiddenNodes2; j++){
        fputs ("[ ", stdout);
        for(int k = 0; k < numHiddenNodes; k++){
            printf("%f ", hiddenWeights2[k][j]);
        }
        fputs("] ", stdout);
    }

    fputs ("]\nFinal Hidden Biases\n[ ", stdout);
    for(int j = 0; j < numHiddenNodes; j++){
        printf("%f ", hiddenLayerBias[j]);
    }

    fputs ("]\nFinal Hidden2 Biases\n[ ", stdout);
    for(int j = 0; j < numHiddenNodes2; j++){
        printf("%f ", hiddenLayerBias2[j]);
    }

    fputs ("]\nFinal Output Weights\n", stdout);
    for(int j = 0; j < numOutputs; j++){
        fputs ("[ ", stdout);
        for(int k = 0; k < numHiddenNodes2; k++){
            printf("%f ", outputWeights[k][j]);
        }
        fputs("] \n", stdout);
    }

    fputs ("\nFinal Output Biases\n[ ", stdout);
    for(int j = 0; j < numOutputs; j++){
        printf("%f ", outputLayerBias[j]);
    }

    fputs("] \n", stdout);
*/

    // Building neural network with the trained weights and bias
    // initialize testInput and testResults
    double testInput[numTest];
    double testResults[numTest];

    // looping through the matrix and sending in one vector at a time to evaluate
    #pragma omp parallel for num_threads(thread_count) shared(testInput, testResults) private(j)
    for(int i = 0; i < numTest; i++) {
        for (int j = 0; j < numInputs; j++) {
            testInput[j] = testingInputs[i][j];
            // printf("%f ", testInput[j]);
        }
        // printf("\n");


        // predicted solution
        testResults[i] = evaluation(numInputs, numHiddenNodes, numHiddenNodes2, numOutputs,
                                    testInput,hiddenWeights,hiddenWeights2,outputWeights,hiddenLayerBias,hiddenLayerBias2,outputLayerBias);
        printf("predicted results: %f   actual result: %f \n", testResults[i], testingOutputs[i]);
    }

    accuracy(testResults,testingOutputs,numTest);             // accuracy, precision, fscore

    double duration = ((double)end - start)/CLOCKS_PER_SEC;
    printf("Time taken to execute in seconds : %f", duration);

    // Now you can access and use the dynamically allocated arrays like regular arrays
    // Don't forget to free the dynamically allocated memory when you're done using it
    for (int i = 0; i < numInputs; i++) {
        free(hiddenWeights[i]);
        free(hiddenWeights2[i]);
    }

    for (int i = 0; i < numHiddenNodes2; i++) {
        free(outputWeights[i]);
    }

    free(hiddenLayer);
    free(hiddenLayer2);
    free(outputLayer);
    free(hiddenLayerBias);
    free(hiddenLayerBias2);
    free(outputLayerBias);
    free(hiddenWeights);
    free(hiddenWeights2);
    free(outputWeights);


    // Free dynamically allocated memory for training and testing inputs and outputs
    for (int i = 0; i < numTrain; i++) {
        free(trainingInputs[i]);
    }
    free(trainingInputs);

    for (int i = 0; i < numTest; i++) {
        free(testingInputs[i]);
    }
    free(testingInputs);

    for (int i = 0; i < numTrain; i++) {
        free(trainingOutputs[i]);
    }
    free(trainingOutputs);

    free(testingOutputs);
    return 0;

}
