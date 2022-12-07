// import java.util.Random;

public class TestGen {
  public static void main(String[] args) {
    // AddTests.makeAddTests();
    // LeftShiftTests.makeTests();
    int a =2147483647&2147483647;
    System.out.println(a);
  }
}

// class AddTests {
//     static Random rand = new Random();
//     static int upperbound = Integer.MAX_VALUE;
//     static int a = 0;
//     static int b = 0;
//     static int cin=0;
//     static long c=0;
//     static int v=0;

//     public static void makeAddTests(){
//       for(int i = 0; i < 50; i++){
//         a = rand.nextInt(upperbound);
//         b = rand.nextInt(upperbound);
//         cin = rand.nextInt(2);
//         c = (long) (a) + (long)(b) + (long)(cin);
//         if (c > (long)(Integer.MAX_VALUE)){
//           v = 1;
//         } else v = 0;
//         System.out.println(a+" " + b + " " + cin + " " + (a+b+cin) + " " + v );
//       }
//     }
// }

// class LeftShiftTests {
//   static Random rand = new Random();
//   static int upperbound = Integer.MAX_VALUE;
//   static int b = 0;
//   static int sa=0;
//   static int c=0;

//   public static void makeTests(){
//     for(int i = 0; i < 50; i++){
//       b = rand.nextInt(upperbound);
//       sa = rand.nextInt(32);
//       c = b << sa;
//       System.out.println( b + " " + sa + " " + 0 + " " + c );
//     }
//   }
// }
