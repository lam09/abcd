package com.ag.game;

public class card {

    public static card instance = new card();
    // static card x10 = new card();

    // declaration of native method
    public native boolean Initialize();
    public native int GetAcceptorAddressBill();
    public native int GetAcceptorAddressCoin();

    public native boolean EnableAcceptorBill();
    public native boolean EnableAcceptorCoin();

    public native int[] GetButtons();
    public native int getLastCoin();
    public native int[] getLastBill();
    public native boolean close();

    public native void controlBrightness(int on, int off, int brightness);
    public native void pulseStart(int output, int duration);
    public native int pulseRemaining();

//    public native void 

    public static final int COUNTER_PULSE_DURATION      = 20;

    public static final int OUTPUT_BIT_BUTTON_MINUS     = 25;
    public static final int OUTPUT_BIT_BUTTON_PLUS      = 23;
    public static final int OUTPUT_BIT_BUTTON_AUTOSTART = 22;
    public static final int OUTPUT_BIT_BUTTON_START     = 24;
    public static final int OUTPUT_BIT_BUTTON_BACK      = 20;
    public static final int OUTPUT_BIT_BUTTON_MAXBET    = 16;

    public static final int OUTPUT_BIT_COUNTER0         = 0;
    public static final int OUTPUT_BIT_COUNTER1         = 1;
    public static final int OUTPUT_BIT_COUNTER2         = 2;

    public static final int OUTPUT_BUTTON_MINUS         = 1<<25;
    public static final int OUTPUT_BUTTON_PLUS          = 1<<23;
    public static final int OUTPUT_BUTTON_AUTOSTART     = 1<<22;
    public static final int OUTPUT_BUTTON_START         = 1<<24;
    public static final int OUTPUT_BUTTON_BACK          = 1<<20;
    public static final int OUTPUT_BUTTON_MAXBET        = 1<<16;


    public card() {
        try {
            System.load("/home/administrator/SlotGameJni/com_ag_game_card.so");
        } catch (UnsatisfiedLinkError e) {
            System.err.println("Native code library failed to load.\n" + e);
            System.exit(1);
        }
    }

    public void dispose() {
        close();
    }

    public static void rollCounter(int counter) {
        instance.pulseStart(counter, COUNTER_PULSE_DURATION);
        // int remaining = instance.pulseRemaining();
        // if (remaining > 0) {
            // try {
                // Thread.sleep(remaining);
            // } catch (Exception ex) {}
        // }
        // while (instance.pulseRemaining() >= 0);
        // try {
            // Thread.sleep(10);
        // } catch (Exception ex) {}
    }

    public static void TestROM(){
       // instance.WriteCheckSum();
        instance.getLastBill();
    }

    public static void main(String[] args) {
          System.out.println("Main method");

 instance.Initialize();


System.out.println("\n"+"coin adresa: " + instance.GetAcceptorAddressCoin());
System.out.println("\n"+"bill adresa: " + instance.GetAcceptorAddressBill());


        int[] inputs;
        int[] lastCoin;
        for (int i=0; i<0; i++) {
            //inputs = instance.GetButtons();
            //for (int j=0; j<inputs.length; j++) {
            //    System.out.print(Integer.toHexString(inputs[j])+" ");
            //}
            //System.out.println();

            lastCoin = instance.getLastBill();

            if (lastCoin == null) { 
                
                System.out.println("\n"+"initializing: NULLPTR");
                continue;

            }

            if (lastCoin[0] > 0) {
                System.out.println("\n"+"bill value: "+lastCoin[0]+" id: " +lastCoin[1]);
                i++;
            } else if (lastCoin[0] == 0) {
                System.out.print(".");
            } else if (lastCoin[0] < 0) {
                System.out.println("\n"+"chyba: "+(-lastCoin[0]));
            }
            try {
                Thread.sleep(2000);
            } catch (InterruptedException ex) {
            }
        }

       

        //TestROM(); //initialize
        //TestROM();


/* test brightness */
/*
        int on;
        int off;
        //on = 14200722;
        //off = 10963604;
        on = 0;
        off = 0;

        for (String arg: args) {
            on += 1<<Integer.parseInt(arg);
            System.out.println(arg);
        }

        System.out.println(on);

        try {
            for (int i=0; i<10; i++) {
                instance.controlBrightness(on, off, i);
                Thread.sleep(100);
            }
            Thread.sleep(1000);
            for (int i=0; i<10; i++) {
                instance.controlBrightness(on, off, 9-i);
                Thread.sleep(50);
            }
        } catch (Exception ex) {}
*/


        // int bit = OUTPUT_BIT_BUTTON_MINUS;
        // if (args.length > 0) {
            // bit = Integer.parseInt(args[0]);
        // }
        // instance.pulseStart(bit, 100);
        // int remaining = 0;
        // while (remaining >= 0) {
            // remaining = instance.pulseRemaining();
            // System.out.println(""+remaining);
        // }

       /* for (int i=0; i<10; i++) {
            //rollCounter(OUTPUT_BIT_COUNTER1);
        }*/

        // if(Ag.LiveBuild){
            // int[] inputs = x10.GetButtons();
            // return inputs;
        // } else {
            // int[ ] inputs = { 128,255 };
            // return inputs;
        // }
    }
}
